/**
 *  @file   larpandoracontent/LArTwoDReco/LArClusterSplitting/TrackConsolidationAlgorithm.h
 *
 *  @brief  Header file for the track consolidation algorithm class.
 *
 *  $Log: $
 */
#ifndef LAR_TRACK_CONSOLIDATION_ALGORITHM_H
#define LAR_TRACK_CONSOLIDATION_ALGORITHM_H 1

#include "larpandoracontent/LArTwoDReco/LArClusterSplitting/TwoDSlidingFitConsolidationAlgorithm.h"

#include "larpandoracontent/LArHelpers/LArClusterHelper.h"

namespace lar_content
{

/**
 *  @brief  TrackConsolidationAlgorithm class
 */
class TrackConsolidationAlgorithm : public TwoDSlidingFitConsolidationAlgorithm
{
public:
    /**
     *  @brief  Default constructor
     */
    TrackConsolidationAlgorithm();

private:
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    /**
     *  @brief Get the list of hits to be added to track clusters and removed from shower clusters
     *
     *  @param slidingFitResultList  the list of sliding linear fits to track clusters
     *  @param showerClusters  the vector of shower clusters
     *  @param caloHitsToAdd   the output map of hits to be added to clusters
     *  @param caloHitsToRemove   the output map of hits to be removed from clusters
     */
    void GetReclusteredHits(const TwoDSlidingFitResultList &slidingFitResultList, const pandora::ClusterVector &showerClusters,
        ClusterToHitMap &caloHitsToAdd, ClusterToHitMap &caloHitsToRemove) const;

    /**
     *  @brief Get the list of hits to be added to a track cluster and removed from a shower cluster
     *
     *  @param slidingFitResult  sliding linear fit to track cluster
     *  @param pTargetCluster  shower cluster
     *  @param caloHitsToAdd  the output map of hits to be added to clusters
     *  @param caloHitsToRemove  the output map of hits to be removed from clusters
     */
    void GetReclusteredHits(const TwoDSlidingFitResult &slidingFitResult, const pandora::Cluster *const pTargetCluster,
        ClusterToHitMap &caloHitsToAdd, ClusterToHitMap &caloHitsToRemove) const;

    /**
     *  @brief Check cross TPC volume cluster associations to look for overlap in drift time (hits deposited in different volumes in
     *  overlapping drift windows cannot be from the same trajectory) and remove impossible associations
     *
     *  @param pCluster1 The first cluster under consideration for merging
     *  @param pCluster2 The second cluster under consideration for merging
     *
     *  @return true if the clusters are from a common volume, false otherwise
     */
    bool CheckInterTPCVolumeAssociations(const pandora::Cluster *const pCluster1, const pandora::Cluster *const pCluster2) const;

    float m_maxTransverseDisplacement; ///<
    float m_minAssociatedSpan;         ///<
    float m_minAssociatedFraction;     ///<
    bool m_checkInterTPCVolumeAssociations; ///< Whether to check for mixing among tpc volumes
};

} // namespace lar_content

#endif // #ifndef LAR_TRACK_CONSOLIDATION_ALGORITHM_H
