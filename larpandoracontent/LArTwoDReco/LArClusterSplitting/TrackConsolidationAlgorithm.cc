/**
 *  @file   larpandoracontent/LArTwoDReco/LArClusterSplitting/TrackConsolidationAlgorithm.cc
 *
 *  @brief  Implementation of the track consolidation algorithm class.
 *
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "larpandoracontent/LArObjects/LArCaloHit.h"
#include "larpandoracontent/LArTwoDReco/LArClusterSplitting/TrackConsolidationAlgorithm.h"

using namespace pandora;

namespace lar_content
{

TrackConsolidationAlgorithm::TrackConsolidationAlgorithm() :
    m_maxTransverseDisplacement(1.f),
    m_minAssociatedSpan(1.f),
    m_minAssociatedFraction(0.5f),
    m_checkInterTPCVolumeAssociations(false)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

void TrackConsolidationAlgorithm::GetReclusteredHits(const TwoDSlidingFitResultList &slidingFitResultListI,
    const ClusterVector &showerClustersJ, ClusterToHitMap &caloHitsToAddI, ClusterToHitMap &caloHitsToRemoveJ) const
{
    for (const TwoDSlidingFitResult &slidingFitResultI : slidingFitResultListI)
    {
        const Cluster *const pClusterI = slidingFitResultI.GetCluster();
        const float thisLengthSquaredI(LArClusterHelper::GetLengthSquared(pClusterI));

        for (const Cluster *const pClusterJ : showerClustersJ)
        {
            const float thisLengthSquaredJ(LArClusterHelper::GetLengthSquared(pClusterJ));

            if (pClusterI == pClusterJ)
                continue;

            if (2.f * thisLengthSquaredJ > thisLengthSquaredI)
                continue;

            this->GetReclusteredHits(slidingFitResultI, pClusterJ, caloHitsToAddI, caloHitsToRemoveJ);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void TrackConsolidationAlgorithm::GetReclusteredHits(const TwoDSlidingFitResult &slidingFitResultI, const Cluster *const pClusterJ,
    ClusterToHitMap &caloHitsToAddI, ClusterToHitMap &caloHitsToRemoveJ) const
{
    const Cluster *const pClusterI(slidingFitResultI.GetCluster());

    if (m_checkInterTPCVolumeAssociations && !this->CheckInterTPCVolumeAssociations(pClusterI, pClusterJ))
        return;

    CaloHitList associatedHits, caloHitListJ;
    pClusterJ->GetOrderedCaloHitList().FillCaloHitList(caloHitListJ);

    float minL(std::numeric_limits<float>::max());
    float maxL(std::numeric_limits<float>::max());

    // Loop over hits from shower clusters, and make associations with track clusters
    // (Determine if hits from shower clusters can be used to fill gaps in track cluster)
    //
    // Apply the following selection:
    //  rJ = candidate hit from shower cluster
    //  rI = nearest hit on track cluster
    //  rK = projection of shower hit onto track cluster
    //
    //                  o rJ
    //  o o o o o o - - x - - - - o o o o o o o
    //           rI    rK
    //
    //  Require: rJK < std::min(rCut, rIJ, rKI)

    for (CaloHitList::const_iterator iterJ = caloHitListJ.begin(), iterEndJ = caloHitListJ.end(); iterJ != iterEndJ; ++iterJ)
    {
        const CaloHit *const pCaloHitJ = *iterJ;

        const CartesianVector positionJ(pCaloHitJ->GetPositionVector());
        const CartesianVector positionI(LArClusterHelper::GetClosestPosition(positionJ, pClusterI));

        float rL(0.f), rT(0.f);
        CartesianVector positionK(0.f, 0.f, 0.f);

        if (STATUS_CODE_SUCCESS != slidingFitResultI.GetGlobalFitProjection(positionJ, positionK))
            continue;

        slidingFitResultI.GetLocalPosition(positionK, rL, rT);

        const float rsqIJ((positionI - positionJ).GetMagnitudeSquared());
        const float rsqJK((positionJ - positionK).GetMagnitudeSquared());
        const float rsqKI((positionK - positionI).GetMagnitudeSquared());

        if (rsqJK < std::min(m_maxTransverseDisplacement * m_maxTransverseDisplacement, std::min(rsqIJ, rsqKI)))
        {
            if (associatedHits.empty())
            {
                minL = rL;
                maxL = rL;
            }
            else
            {
                minL = std::min(minL, rL);
                maxL = std::max(maxL, rL);
            }

            associatedHits.push_back(pCaloHitJ);
        }
    }

    const float associatedSpan(maxL - minL);
    const float associatedFraction(
        associatedHits.empty() ? 0.f : static_cast<float>(associatedHits.size()) / static_cast<float>(pClusterJ->GetNCaloHits()));

    if (associatedSpan > m_minAssociatedSpan || associatedFraction > m_minAssociatedFraction)
    {
        for (CaloHitList::const_iterator iterK = associatedHits.begin(), iterEndK = associatedHits.end(); iterK != iterEndK; ++iterK)
        {
            const CaloHit *const pCaloHit = *iterK;
            const CaloHitList &caloHitList(caloHitsToRemoveJ[pClusterJ]);

            if (caloHitList.end() != std::find(caloHitList.begin(), caloHitList.end(), pCaloHit))
                continue;

            caloHitsToAddI[pClusterI].push_back(pCaloHit);
            caloHitsToRemoveJ[pClusterJ].push_back(pCaloHit);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

bool TrackConsolidationAlgorithm::CheckInterTPCVolumeAssociations(const Cluster *const pCluster1, const Cluster *const pCluster2) const
{
    CaloHitList caloHitList1;
    pCluster1->GetOrderedCaloHitList().FillCaloHitList(caloHitList1);
    if (caloHitList1.empty())
        return true;
    // ATTN: Early 2D clustering should preclude input clusters containing mixed volumes, so just check the first hit
    const LArCaloHit *const pLArCaloHit1{dynamic_cast<const LArCaloHit *const>(caloHitList1.front())};
    if (!pLArCaloHit1)
        return true;
    const unsigned int clusterTpcVolume1{pLArCaloHit1->GetLArTPCVolumeId()};
    const unsigned int clusterSubVolume1{pLArCaloHit1->GetSubVolumeId()};

    CaloHitList caloHitList2;
    pCluster2->GetOrderedCaloHitList().FillCaloHitList(caloHitList2);
    if (caloHitList2.empty())
        return true;
    const LArCaloHit *const pLArCaloHit2{dynamic_cast<const LArCaloHit *const>(caloHitList2.front())};
    if (!pLArCaloHit2)
        return true;
    const unsigned int clusterTpcVolume2{pLArCaloHit2->GetLArTPCVolumeId()};
    const unsigned int clusterSubVolume2{pLArCaloHit2->GetSubVolumeId()};

    if (clusterTpcVolume1 == clusterTpcVolume2 && clusterSubVolume1 == clusterSubVolume2)
    {
        // Same volume, no problem
        return true;
    }
    else
    {
        // Volumes differ, veto
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode TrackConsolidationAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=,
        XmlHelper::ReadValue(xmlHandle, "MaxTransverseDisplacement", m_maxTransverseDisplacement));

    PANDORA_RETURN_RESULT_IF_AND_IF(
        STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "MinAssociatedSpan", m_minAssociatedSpan));

    PANDORA_RETURN_RESULT_IF_AND_IF(
        STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "MinAssociatedFraction", m_minAssociatedFraction));

    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=,
        XmlHelper::ReadValue(xmlHandle, "CheckInterTPCVolumeAssociations", m_checkInterTPCVolumeAssociations));

    //std::cout << "[TrackConsolidationAlgorithm] CHECKING INTER TPC VOLUME ASSNS? --> " << m_checkInterTPCVolumeAssociations << std::endl;

    return TwoDSlidingFitConsolidationAlgorithm::ReadSettings(xmlHandle);
}

} // namespace lar_content
