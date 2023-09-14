/**
 *  @file   larpandoracontent/LArUtility/CheckClusterSubVolume.cc
 *
 *  @brief  Dump calo hit info...
 
 *  $Log: $
 */

// BH: Big thanks to Andy C. Copied from one of my other test branches and edited to be what I want on this branch.

#include "Pandora/AlgorithmHeaders.h"

#include "larpandoracontent/LArUtility/CheckClusterSubVolume.h"

#include "larpandoracontent/LArHelpers/LArClusterHelper.h"
#include "larpandoracontent/LArObjects/LArCaloHit.h"

#include <set>

using namespace pandora;

namespace lar_content
{

  CheckClusterSubVolume::CheckClusterSubVolume() : m_useSpecificList(false)
  {
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode CheckClusterSubVolume::Run()
  {
    //std::cout << "\n\n" << std::endl;
    std::cout << "CheckClusterSubVolume :: Auxiliary info: " << m_auxiliaryInfo << std::endl;

    // Get List of Clusters to check
    const ClusterList *pClusterList = NULL;
    if ( !m_useSpecificList ) {
      std::cout << "Using current list..." << std::endl;
      PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));
    }
    else {
      std::cout << "Looking for list " << m_clusterListName << "..." << std::endl;
      PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetList(*this, m_clusterListName, pClusterList));
    }

    std::cout << "CHECKING " << pClusterList->size() << " CLUSTERS" << std::endl;
    if ( pClusterList->size() == 0 ) {
      std::cout << "No clusters... Returning" << std::endl;
      return STATUS_CODE_SUCCESS;
    }

    // Loop through the clusters and perform the check
    auto itCluster = pClusterList->begin();
    unsigned int numCluster(0);
    while ( itCluster!=pClusterList->end() ) {
      const Cluster *pCluster = *itCluster;
      std::map< std::pair<unsigned int,unsigned int>, unsigned int > volumeHitsMap;

      CaloHitList caloHitList;
      pCluster->GetOrderedCaloHitList().FillCaloHitList(caloHitList);
      if (caloHitList.empty()) {
	++numCluster;
	continue;
      }

      for ( auto const& pCaloHit : caloHitList ) {
	const LArCaloHit *const pLArCaloHit{dynamic_cast<const LArCaloHit *const>(pCaloHit)};
	if (!pLArCaloHit)
	  continue;

	const unsigned int clusterTpcVolume{pLArCaloHit->GetLArTPCVolumeId()};
	const unsigned int clusterSubVolume{pLArCaloHit->GetSubVolumeId()};

	if ( volumeHitsMap.find( std::make_pair(clusterTpcVolume,clusterSubVolume) ) == volumeHitsMap.end() ) {
	  volumeHitsMap[ std::make_pair(clusterTpcVolume,clusterSubVolume) ] = 1;
	}
	else {
	  volumeHitsMap.at( std::make_pair(clusterTpcVolume,clusterSubVolume) ) += 1;
	}
      } // calo hits

      if ( volumeHitsMap.size() > 1 ) {
	std::cout << "Cluster " << numCluster << ":" << std::endl;
	for ( auto const &[key, value] : volumeHitsMap ) {
	  std::cout << "  " << key.first << ":" << key.second << " = " << value << std::endl;
	}
      }

      ++numCluster;
      ++itCluster;
    } // clusters

    //std::cout << "\n\n" << std::endl;

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode CheckClusterSubVolume::ReadSettings(const TiXmlHandle xmlHandle)
  {
    // Auxiliary info to print
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "AuxInfo", m_auxiliaryInfo));

    // Use specific list?
    PANDORA_RETURN_RESULT_IF_AND_IF(STATUS_CODE_SUCCESS, STATUS_CODE_NOT_FOUND, !=, XmlHelper::ReadValue(xmlHandle, "NonCurrent", m_useSpecificList));
    // std::cout << "Use list other than current? " << (m_useSpecificList ? "Yes" : "No, use current.") << std::endl;
    if ( m_useSpecificList ) {
      PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle, "ClusterListName", m_clusterListName));
    }

    return STATUS_CODE_SUCCESS;
  }

} // namespace lar_content
