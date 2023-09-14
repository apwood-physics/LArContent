/**
 *  @file   larpandoracontent/LArUtility/DaughterVolumeCatalogAlgorithm.h
 *
 *  @brief  Header file for the list pruning algorithm class.
 *
 *  $Log: $
 */
#ifndef LAR_DAUGHTER_VOLUME_CATALOG_H
#define LAR_DAUGHTER_VOLUME_CATALOG_H 1

#include "Pandora/Algorithm.h"

namespace lar_content
{

/**
 *  @brief  DaughterVolumeCatalogAlgorithm class
 */
class DaughterVolumeCatalogAlgorithm : public pandora::Algorithm
{
public:
    /**
     *  @brief  Default constructor
     */
    DaughterVolumeCatalogAlgorithm();

private:
    pandora::StatusCode Run();
    pandora::StatusCode ProcessHits();
    pandora::StatusCode ProcessClusters();
    pandora::StatusCode ProcessPfos();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    bool m_mergeDriftVolume;        //< Whether or not to combined daughter volumes within a drift volume
    bool m_processHits;             //< Whether or not to process hit-level information
    bool m_processClusters;         //< Whether or not to process cluster-level information
    bool m_processPfos;             //< Whether or not to process PFO-level information
    std::string m_clusterListName;  //< The name of the cluster list, if processing clusters
    std::string m_pfoListName;      //< The name of the PFO list, if processing PFOs
};

} // namespace lar_content

#endif // #ifndef LAR_DAUGHTER_VOLUME_CATALOG_H
