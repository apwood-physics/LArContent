/**
 *  @file   larpandoracontent/LArTwoDReco/LArClusterAssociation/ClusterExtensionAlgorithm.cc
 *
 *  @brief  Implementation of the cluster extension algorithm class.
 *
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "larpandoracontent/LArObjects/LArCaloHit.h"

#include "larpandoracontent/LArTwoDReco/LArClusterAssociation/ClusterExtensionAlgorithm.h"

using namespace pandora;

namespace lar_content
{

ClusterExtensionAlgorithm::ClusterExtensionAlgorithm() : m_checkInterTPCVolumeAssociations(false)
{
}

void ClusterExtensionAlgorithm::PopulateClusterMergeMap(const ClusterVector &clusterVector, ClusterMergeMap &clusterMergeMap) const
{
    ClusterAssociationMatrix clusterAssociationMatrix;
    this->FillClusterAssociationMatrix(clusterVector, clusterAssociationMatrix);
    if( m_checkInterTPCVolumeAssociations ) this->CheckInterTPCVolumeAssociations(clusterAssociationMatrix);
    this->FillClusterMergeMap(clusterAssociationMatrix, clusterMergeMap);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ClusterExtensionAlgorithm::CheckInterTPCVolumeAssociations(ClusterAssociationMatrix &clusterAssociationMatrix) const
{
    ClusterSet unassociatedClusters;
    for (auto & [ pCluster, clusterAssociationMap ] : clusterAssociationMatrix)
    {
        CaloHitList caloHitList;
        pCluster->GetOrderedCaloHitList().FillCaloHitList(caloHitList);
        if (caloHitList.empty())
            continue;
        // ATTN: Early 2D clustering should preclude input clusters containing mixed volumes, so just check the first hit
        const LArCaloHit *const pLArCaloHit{dynamic_cast<const LArCaloHit *const>(caloHitList.front())};
        if (!pLArCaloHit)
            continue;
        const unsigned int clusterTpcVolume{pLArCaloHit->GetLArTPCVolumeId()};
        const unsigned int clusterSubVolume{pLArCaloHit->GetSubVolumeId()};

        auto assocIter{clusterAssociationMap.begin()};
        while (assocIter != clusterAssociationMap.end())
        {
            const Cluster *pOtherCluster{static_cast<const Cluster *>(assocIter->first)};
            CaloHitList otherHitList;
            pOtherCluster->GetOrderedCaloHitList().FillCaloHitList(otherHitList);
            if (otherHitList.empty())
                continue;
            const LArCaloHit *const pLArOtherHit{dynamic_cast<const LArCaloHit *const>(otherHitList.front())};
            if (!pLArOtherHit)
                continue;
            const unsigned int otherTpcVolume{pLArOtherHit->GetLArTPCVolumeId()};
            const unsigned int otherSubVolume{pLArOtherHit->GetSubVolumeId()};

            if (clusterTpcVolume == otherTpcVolume && clusterSubVolume == otherSubVolume)
            {
                // Same volume, move on
                ++assocIter;
            }
            else
            {
                // Volumes differ, veto
                assocIter = clusterAssociationMap.erase(assocIter);
            }
        }
        if (clusterAssociationMap.empty())
            unassociatedClusters.insert(pCluster);
    }

    for (const Cluster *const pCluster : unassociatedClusters)
        clusterAssociationMatrix.erase(pCluster);
}

StatusCode ClusterExtensionAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=,
        XmlHelper::ReadValue(xmlHandle, "CheckInterTPCVolumeAssociations", m_checkInterTPCVolumeAssociations));

    //std::cout << "[ClusterExtensionAlgorithm] CHECKING INTER TPC VOLUME ASSNS? --> " << m_checkInterTPCVolumeAssociations << std::endl;

    return ClusterMergingAlgorithm::ReadSettings(xmlHandle);
}

} // namespace lar_content
