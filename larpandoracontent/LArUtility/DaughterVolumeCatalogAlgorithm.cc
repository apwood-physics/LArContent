/**
 *  @file   larpandoracontent/LArUtility/DaughterVolumeCatalogAlgorithm.cc
 *
 *  @brief  Implementation of the list pruning algorithm class.
 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "larpandoracontent/LArUtility/DaughterVolumeCatalogAlgorithm.h"

#include "larpandoracontent/LArHelpers/LArPfoHelper.h"
#include "larpandoracontent/LArObjects/LArCaloHit.h"

using namespace pandora;

namespace lar_content
{

DaughterVolumeCatalogAlgorithm::DaughterVolumeCatalogAlgorithm() :
    m_mergeDriftVolume{true},
    m_processHits{false},
    m_processClusters{false},
    m_processPfos{false}
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode DaughterVolumeCatalogAlgorithm::Run()
{
    if (m_processHits)
        return this->ProcessHits();
    if (m_processClusters)
        return this->ProcessClusters();
    if (m_processPfos)
        return this->ProcessPfos();

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode DaughterVolumeCatalogAlgorithm::ProcessHits()
{
    const CaloHitList *pCaloHitList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, "CaloHitList2D", pCaloHitList));

    PANDORA_MONITORING_API(SetEveDisplayParameters(this->GetPandora(), true, DETECTOR_VIEW_XZ, -1.f, 1.f, 1.f));

    std::map<std::string, CaloHitList> volumeToHitsMap;
    for (const CaloHit *const pCaloHit : *pCaloHitList)
    {
        const LArCaloHit *const pLArCaloHit{dynamic_cast<const LArCaloHit *const>(pCaloHit)};
        if (!pLArCaloHit)
            continue;
        const HitType view{pLArCaloHit->GetHitType()};
        const unsigned int tpcVolume{pLArCaloHit->GetLArTPCVolumeId()};
        // ATTN - Drift volume merging assumes consecutive daughter volume IDs are stacked, this isn't a generic solution
        const unsigned int daughterVolume{m_mergeDriftVolume ? pLArCaloHit->GetDaughterVolumeId() % 2 : pLArCaloHit->GetDaughterVolumeId()};
        std::string key{std::to_string(tpcVolume) + "_" + std::to_string(daughterVolume) + "_" + std::to_string(view)};
        if (volumeToHitsMap.find(key) == volumeToHitsMap.end())
            volumeToHitsMap[key] = CaloHitList();
        volumeToHitsMap[key].emplace_back(pLArCaloHit);
    }

    for (const auto & [ key, hits ] : volumeToHitsMap)
    {
        PANDORA_MONITORING_API(VisualizeCaloHits(this->GetPandora(), &hits, key, AUTOITER));
    }

    PANDORA_MONITORING_API(ViewEvent(this->GetPandora()));

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode DaughterVolumeCatalogAlgorithm::ProcessClusters()
{
    const ClusterList *pClusterList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_clusterListName, pClusterList));

    PANDORA_MONITORING_API(SetEveDisplayParameters(this->GetPandora(), true, DETECTOR_VIEW_XZ, -1.f, 1.f, 1.f));

    std::map<std::string, CaloHitList> volumeToHitsMap;
    int i{0};
    for (const Cluster *const pCluster : *pClusterList)
    {
        std::map<std::string, bool> clusterVolumesMap;
        CaloHitList allHits;
        pCluster->GetOrderedCaloHitList().FillCaloHitList(allHits);
        const CaloHitList &isoHits{pCluster->GetIsolatedCaloHitList()};
        allHits.insert(allHits.end(), isoHits.begin(), isoHits.end());
        for (const CaloHit *const pCaloHit : allHits)
        {
            const LArCaloHit *const pLArCaloHit{dynamic_cast<const LArCaloHit *const>(pCaloHit)};
            if (!pLArCaloHit)
                continue;
            const HitType view{pLArCaloHit->GetHitType()};
            const unsigned int tpcVolume{pLArCaloHit->GetLArTPCVolumeId()};
            // ATTN - Drift volume merging assumes consecutive daughter volume IDs are stacked, this isn't a generic solution
            const unsigned int daughterVolume{m_mergeDriftVolume ? pLArCaloHit->GetDaughterVolumeId() % 2 : pLArCaloHit->GetDaughterVolumeId()};
            std::string key{std::to_string(i) + "_" + std::to_string(tpcVolume) + "_" + std::to_string(daughterVolume) + "_" + std::to_string(view)};
            clusterVolumesMap[key] = true;
        }
        if (clusterVolumesMap.size() > 1)
        {
            for (const CaloHit *const pCaloHit : allHits)
            {
                const LArCaloHit *const pLArCaloHit{dynamic_cast<const LArCaloHit *const>(pCaloHit)};
                if (!pLArCaloHit)
                    continue;
                const HitType view{pLArCaloHit->GetHitType()};
                const unsigned int tpcVolume{pLArCaloHit->GetLArTPCVolumeId()};
                // ATTN - Drift volume merging assumes consecutive daughter volume IDs are stacked, this isn't a generic solution
                const unsigned int daughterVolume{m_mergeDriftVolume ? pLArCaloHit->GetDaughterVolumeId() % 2 : pLArCaloHit->GetDaughterVolumeId()};
                std::string key{std::to_string(i) + "_" + std::to_string(tpcVolume) + "_" + std::to_string(daughterVolume) + "_" + std::to_string(view)};
                if (volumeToHitsMap.find(key) == volumeToHitsMap.end())
                    volumeToHitsMap[key] = CaloHitList();
                volumeToHitsMap[key].emplace_back(pLArCaloHit);
            }
        }
        ++i;
    }

    for (const auto & [ key, hits ] : volumeToHitsMap)
    {
        std::cout << key << " " << hits.size() <<std::endl;
        PANDORA_MONITORING_API(VisualizeCaloHits(this->GetPandora(), &hits, key, AUTOITER));
    }

    PANDORA_MONITORING_API(ViewEvent(this->GetPandora()));

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode DaughterVolumeCatalogAlgorithm::ProcessPfos()
{
    const PfoList *pPfoList(nullptr);
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_pfoListName, pPfoList));

    PANDORA_MONITORING_API(SetEveDisplayParameters(this->GetPandora(), true, DETECTOR_VIEW_XZ, -1.f, 1.f, 1.f));

    std::map<std::string, CaloHitList> volumeToHitsMap;
    int i{0};
    for (const ParticleFlowObject *const pPfo : *pPfoList)
    {
        CaloHitList allHits;
        LArPfoHelper::GetAllCaloHits(pPfo, allHits);
        for (const CaloHit *const pCaloHit : allHits)
        {
            const LArCaloHit *const pLArCaloHit{dynamic_cast<const LArCaloHit *const>(pCaloHit)};
            if (!pLArCaloHit)
                continue;
            const HitType view{pLArCaloHit->GetHitType()};
            const unsigned int tpcVolume{pLArCaloHit->GetLArTPCVolumeId()};
            // ATTN - Drift volume merging assumes consecutive daughter volume IDs are stacked, this isn't a generic solution
            const unsigned int daughterVolume{m_mergeDriftVolume ? pLArCaloHit->GetDaughterVolumeId() % 2 : pLArCaloHit->GetDaughterVolumeId()};
            std::string key{std::to_string(i) + "_" + std::to_string(tpcVolume) + "_" + std::to_string(daughterVolume) + "_" + std::to_string(view)};
            if (volumeToHitsMap.find(key) == volumeToHitsMap.end())
                volumeToHitsMap[key] = CaloHitList();
            volumeToHitsMap[key].emplace_back(pLArCaloHit);
        }
        ++i;
    }

    for (const auto & [ key, hits ] : volumeToHitsMap)
    {
        std::cout << key << " " << hits.size() <<std::endl;
        PANDORA_MONITORING_API(VisualizeCaloHits(this->GetPandora(), &hits, key, AUTOITER));
    }

    PANDORA_MONITORING_API(ViewEvent(this->GetPandora()));

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode DaughterVolumeCatalogAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "MergeDriftVolume", m_mergeDriftVolume));
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "ProcessHits", m_processHits));
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "ProcessClusters", m_processClusters));
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "ProcessPfos", m_processPfos));

    if (m_processClusters)
    {
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "ClusterListName", m_clusterListName));
    }
    if (m_processPfos)
    {
        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "PfoListName", m_pfoListName));
    }

    return STATUS_CODE_SUCCESS;
}

} // namespace lar_content
