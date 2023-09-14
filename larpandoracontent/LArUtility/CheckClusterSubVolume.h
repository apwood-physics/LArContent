/**
 *  @file   larpandoracontent/LArUtility/CheckClusterSubVolume.h
 *
 *  @brief  Header file for the algorithm checking if clusters have hits in multiple subvolumes
 *
 *  $Log: $
 */

// BH, with thanks to Andy Chappell, and based on code originally being used for something else

#ifndef CHECK_CLUSTER_SUB_VOLUME_H
#define CHECK_CLUSTER_SUB_VOLUME_H 1

#include "Pandora/Algorithm.h"

namespace lar_content
{

  /**
   *  @brief CheckClusterSubVolume class
   */
  class CheckClusterSubVolume : public pandora::Algorithm
  {
  public:
    /**
     *  @brief  Default constructor
     */
    CheckClusterSubVolume();

  private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    std::string m_auxiliaryInfo;   ///< String that prints at the beginning of the algorithm so you can tell it where you put it in the sequence...
    bool m_useSpecificList;        ///< Whether to use a custom list name in the check
    std::string m_clusterListName; ///< The cluster list to use if m_UseSpecificList==true
  };

} // namespace lar_content

#endif // #ifndef CHECK_CLUSTER_SUB_VOLUME_H
