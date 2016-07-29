#pragma once

#include <cstdint>
#include <utility>
#include <cmath>
#include <tuple>
#include <vector>
#include <array>
#include <stdexcept>
#include <algorithm>

/// tarsier is a collection of event handlers.
namespace tarsier {

    /// Blob represents a tracked gaussian blob.
    struct Blob {
        double x;
        double y;
        double squaredSigmaX;
        double sigmaXY;
        double squaredSigmaY;
    };

    /// TrackBlobs tracks the incoming events with a static set of blobs.
    /// HandlePromotedBlob must have the signature:
    ///     handlePromotedBlob(std::size_t id, const Blob& blob) -> void
    /// HandleUpdatedBlob must have the signature:
    ///     handleUpdatedBlob(std::size_t id, const Blob& blob) -> void
    /// HandleDemotedBlob must have the signature:
    ///     handleDemotedBlob(std::size_t id, const Blob& blob) -> void
    template <typename Event, typename HandlePromotedBlob, typename HandleUpdatedBlob, typename HandleDemotedBlob, typename HandleDeletedBlob>
    class TrackBlobs {
        public:
            TrackBlobs(
                std::vector<Blob> initialBlobs,
                int64_t initialTimestamp,
                double activityDecay,
                double minimumProbability,
                double promotionActivity,
                double deletionActivity,
                double meanInertia,
                double covarianceInertia,
                double repulsionStrength,
                double repulsionLength,
                double attractionStrength,
                double attractionResetDistance,
                std::size_t pairwiseCalculationsToSkip,
                HandlePromotedBlob handlePromotedBlob,
                HandleUpdatedBlob handleUpdatedBlob,
                HandleDemotedBlob handleDemotedBlob,
                HandleDeletedBlob handleDeletedBlob
            ) :
                _initialBlobs(std::move(initialBlobs)),
                _activityDecay(activityDecay),
                _minimumProbability(minimumProbability),
                _promotionActivity(promotionActivity),
                _deletionActivity(deletionActivity),
                _meanInertia(meanInertia),
                _covarianceInertia(covarianceInertia),
                _repulsionStrength(repulsionStrength),
                _repulsionLength(repulsionLength),
                _attractionStrength(_attractionStrength),
                _attractionResetDistanceSquared(std::pow(attractionResetDistance, 2)),
                _pairwiseCalculationsToSkip(pairwiseCalculationsToSkip),
                _handlePromotedBlob(std::forward<HandlePromotedBlob>(handlePromotedBlob)),
                _handleUpdatedBlob(std::forward<HandleUpdatedBlob>(handleUpdatedBlob)),
                _handleDemotedBlob(std::forward<HandleDemotedBlob>(handleDemotedBlob)),
                _handleDeletedBlob(std::forward<HandleDeletedBlob>(handleDeletedBlob)),
                _previousTimestamp(initialTimestamp),
                _datum(_initialBlobs.size()),
                _idOffset(0)
            {
                for (auto blobIterator = _initialBlobs.begin(); blobIterator != _initialBlobs.end(); ++blobIterator) {
                    _datum[blobIterator - _initialBlobs.begin()].id = blobIterator - _initialBlobs.begin();
                    _datum[blobIterator - _initialBlobs.begin()].blob = *blobIterator;
                    _datum[blobIterator - _initialBlobs.begin()].activity = 0;
                    _datum[blobIterator - _initialBlobs.begin()].status = Status::hidden;
                }
            }
            TrackBlobs(const TrackBlobs&) = delete;
            TrackBlobs(TrackBlobs&&) = default;
            TrackBlobs& operator=(const TrackBlobs&) = delete;
            TrackBlobs& operator=(TrackBlobs&&) = default;
            virtual ~TrackBlobs() {}

            /// operator() handles an event.
            virtual void operator()(Event event) {
                {
                    auto probability = static_cast<double>(0);
                    auto winner = _datum.end();
                    for (auto dataIterator = _datum.begin(); dataIterator != _datum.end(); ++dataIterator) {
                        const auto xPosition = static_cast<double>(event.x) - dataIterator->blob.x;
                        const auto yPosition = static_cast<double>(event.y) - dataIterator->blob.y;
                        const auto determinant = dataIterator->blob.squaredSigmaX * dataIterator->blob.squaredSigmaY - std::pow(dataIterator->blob.sigmaXY, 2);
                        const auto probabilityCandidate =
                            std::exp(-(
                                std::pow(xPosition, 2) * dataIterator->blob.squaredSigmaY
                                + std::pow(yPosition, 2) * dataIterator->blob.squaredSigmaX
                                - 2 * xPosition * yPosition * dataIterator->blob.sigmaXY
                            ) / (2 * determinant))
                            /
                            std::sqrt(determinant)
                        ;
                        if (probabilityCandidate > probability) {
                            probability = probabilityCandidate;
                            winner = dataIterator;
                        }
                    }
                    probability /= (2 * M_PI);

                    const auto exponentialDecay = std::exp(-(event.timestamp - _previousTimestamp) / _activityDecay);
                    auto datumToAdd = std::vector<Data>();
                    for (auto dataIterator = _datum.begin(); dataIterator != _datum.end();) {
                        dataIterator->activity *= exponentialDecay;
                        if (dataIterator == winner && probability > _minimumProbability) {
                            winner->activity += probability;
                            dataIterator->blob.x = _meanInertia * dataIterator->blob.x + (1 - _meanInertia) * static_cast<double>(event.x);
                            dataIterator->blob.y = _meanInertia * dataIterator->blob.y + (1 - _meanInertia) * static_cast<double>(event.y);
                            const auto xPosition = static_cast<double>(event.x) - dataIterator->blob.x;
                            const auto yPosition = static_cast<double>(event.y) - dataIterator->blob.y;
                            dataIterator->blob.squaredSigmaX = _covarianceInertia * dataIterator->blob.squaredSigmaX
                                + (1 - _covarianceInertia) * std::pow(xPosition, 2);
                            dataIterator->blob.sigmaXY = _covarianceInertia * dataIterator->blob.sigmaXY
                                + (1 - _covarianceInertia) * xPosition * yPosition;
                            dataIterator->blob.squaredSigmaY = _covarianceInertia * dataIterator->blob.squaredSigmaY
                                + (1 - _covarianceInertia) * std::pow(yPosition, 2);
                            if (dataIterator->status == Status::promoted) {
                                _handleUpdatedBlob(dataIterator->id, dataIterator->blob);
                            }
                        }

                        switch (dataIterator->status) {
                            case Status::hidden:
                                if (dataIterator->activity > _promotionActivity) {
                                    datumToAdd.push_back(Data{
                                        _idOffset,
                                        dataIterator->blob,
                                        dataIterator->activity,
                                        Status::promoted,
                                    });
                                    _handlePromotedBlob(datumToAdd.back().id, datumToAdd.back().blob);
                                    ++_idOffset;
                                    dataIterator->blob = _initialBlobs[dataIterator->id];
                                    dataIterator->activity = 0;
                                }
                                ++dataIterator;
                                break;
                            case Status::promoted:
                                if (dataIterator->activity <= _promotionActivity) {
                                    if (dataIterator->activity <= _deletionActivity) {
                                        _handleDeletedBlob(dataIterator->id, dataIterator->blob);
                                        dataIterator = _datum.erase(dataIterator);
                                    } else {
                                        _handleDemotedBlob(dataIterator->id, dataIterator->blob);
                                        dataIterator->status = Status::demoted;
                                        ++dataIterator;
                                    }
                                } else {
                                    ++dataIterator;
                                }
                                break;

                            case Status::demoted:
                                if (dataIterator->activity <= _deletionActivity) {
                                    _handleDeletedBlob(dataIterator->id, dataIterator->blob);
                                    dataIterator = _datum.erase(dataIterator);
                                } else if (dataIterator->activity > _promotionActivity) {
                                    _handlePromotedBlob(dataIterator->id, dataIterator->blob);
                                    dataIterator->status = Status::promoted;
                                    ++dataIterator;
                                } else {
                                    ++dataIterator;
                                }
                                break;
                        }
                    }
                    _datum.insert(_datum.end(), datumToAdd.begin(), datumToAdd.end());
                }

                if (_skippedEvents >= _pairwiseCalculationsToSkip) {
                    _skippedEvents = 0;
                    auto xDeltas = std::vector<double>(_datum.size());
                    auto yDeltas = std::vector<double>(_datum.size());
                    for (auto dataIterator = _datum.begin(); dataIterator != _datum.end(); ++dataIterator) {
                        for (auto otherDataIterator = std::next(dataIterator); otherDataIterator != _datum.end(); ++otherDataIterator) {
                            const auto squaredActivity = std::pow(dataIterator->activity, 2);
                            const auto otherSquaredActivity = std::pow(otherDataIterator->activity, 2);
                            const auto activitySum = squaredActivity + otherSquaredActivity;
                            const auto distanceDecay = _repulsionStrength * std::exp(
                                -std::hypot(dataIterator->blob.x - otherDataIterator->blob.x, dataIterator->blob.y - otherDataIterator->blob.y)
                                /
                                _repulsionLength
                            );
                            {
                                const auto activityCorrection = (activitySum == 0 ? 0 : otherSquaredActivity / activitySum);
                                xDeltas[dataIterator - _datum.begin()] -= distanceDecay * activityCorrection * (otherDataIterator->blob.x - dataIterator->blob.x);
                                yDeltas[dataIterator - _datum.begin()] -= distanceDecay * activityCorrection * (otherDataIterator->blob.y - dataIterator->blob.y);
                            }
                            {
                                const auto activityCorrection = (activitySum == 0 ? 0 : squaredActivity / activitySum);
                                xDeltas[otherDataIterator - _datum.begin()] -= distanceDecay * activityCorrection * (dataIterator->blob.x - otherDataIterator->blob.x);
                                yDeltas[otherDataIterator - _datum.begin()] -= distanceDecay * activityCorrection * (dataIterator->blob.y - otherDataIterator->blob.y);
                            }
                        }
                    }
                    std::for_each(_datum.begin(), _datum.end(), [this, &xDeltas, &yDeltas](Data& data) {
                        if (std::pow(data.blob.x, 2) + std::pow(data.blob.y, 2) > _attractionResetDistanceSquared) {
                            xDeltas[data.id] += _attractionStrength * (_initialBlobs[data.id].x - data.blob.x);
                            yDeltas[data.id] += _attractionStrength * (_initialBlobs[data.id].y - data.blob.y);
                        } else {
                            data.blob = _initialBlobs[data.id];
                            data.activity = 0;
                        }
                    });
                    for (auto dataIterator = _datum.begin(); dataIterator != _datum.end(); ++dataIterator) {
                        dataIterator->blob.x += xDeltas[dataIterator - _datum.begin()];
                        dataIterator->blob.y += yDeltas[dataIterator - _datum.begin()];
                    }
                } else {
                    ++_skippedEvents;
                }

                _previousTimestamp = event.timestamp;
            }

        protected:

            /// Status represents the blob lifecycle status.
            enum class Status {
                hidden,
                promoted,
                demoted,
            };

            /// Data represents the algorithm's inner data associated with a blob.
            struct Data {
                std::size_t id;
                Blob blob;
                double activity;
                Status status;
            };

            std::vector<Blob> _initialBlobs;
            const double _activityDecay;
            const double _minimumProbability;
            const double _promotionActivity;
            const double _deletionActivity;
            const double _meanInertia;
            const double _covarianceInertia;
            const double _repulsionStrength;
            const double _repulsionLength;
            const double _attractionStrength;
            const double _attractionResetDistanceSquared;
            const std::size_t _pairwiseCalculationsToSkip;
            HandlePromotedBlob _handlePromotedBlob;
            HandleUpdatedBlob _handleUpdatedBlob;
            HandleDemotedBlob _handleDemotedBlob;
            HandleDeletedBlob _handleDeletedBlob;
            int64_t _previousTimestamp;
            std::size_t _skippedEvents;
            std::vector<Data> _datum;
            std::size_t _idOffset;
    };

    /// make_trackBlobs creates a TrackBlob from functors.
    template <typename Event, typename HandlePromotedBlob, typename HandleUpdatedBlob, typename HandleDemotedBlob, typename HandleDeletedBlob>
    TrackBlobs<Event, HandlePromotedBlob, HandleUpdatedBlob, HandleDemotedBlob, HandleDeletedBlob> make_trackBlobs(
        std::vector<Blob> initialBlobs,
        int64_t initialTimestamp,
        double activityDecay,
        double minimumProbability,
        double promotionActivity,
        double deletionActivity,
        double meanInertia,
        double covarianceInertia,
        double repulsionStrength,
        double repulsionLength,
        double attractionStrength,
        double attractionResetDistance,
        std::size_t pairwiseCalculationsToSkip,
        HandlePromotedBlob handlePromotedBlob,
        HandleUpdatedBlob handleUpdatedBlob,
        HandleDemotedBlob handleDemotedBlob,
        HandleDeletedBlob handleDeletedBlob
    ) {
        return TrackBlobs<Event, HandlePromotedBlob, HandleUpdatedBlob, HandleDemotedBlob, HandleDeletedBlob>(
            std::move(initialBlobs),
            initialTimestamp,
            activityDecay,
            minimumProbability,
            promotionActivity,
            deletionActivity,
            meanInertia,
            covarianceInertia,
            repulsionStrength,
            repulsionLength,
            attractionStrength,
            attractionResetDistance,
            pairwiseCalculationsToSkip,
            std::forward<HandlePromotedBlob>(handlePromotedBlob),
            std::forward<HandleUpdatedBlob>(handleUpdatedBlob),
            std::forward<HandleDemotedBlob>(handleDemotedBlob),
            std::forward<HandleDeletedBlob>(handleDeletedBlob)
        );
    }
}
