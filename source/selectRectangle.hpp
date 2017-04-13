#pragma once

#include <cstdint>
#include <utility>

/// tarsier is a collection of event handlers.
namespace tarsier {

    /// SelectRectangle propagates only the events within the specified rectangular window.
    template <typename Event, uint_fast16_t left, uint_fast16_t bottom, uint_fast16_t width, uint_fast16_t height, typename HandleEvent>
    class SelectRectangle {
        public:
            SelectRectangle(HandleEvent handleEvent) :
                _handleEvent(std::forward<HandleEvent>(handleEvent))
            {
            }
            SelectRectangle(const SelectRectangle&) = delete;
            SelectRectangle(SelectRectangle&&) = default;
            SelectRectangle& operator=(const SelectRectangle&) = delete;
            SelectRectangle& operator=(SelectRectangle&&) = default;
            virtual ~SelectRectangle() {}

            /// operator() handles an event.
            virtual void operator()(const Event& event) {
                if (event.x >= left && event.x < left + width && event.y >= bottom && event.y < bottom + height) {
                    _handleEvent(event);
                }
            }

        protected:
            HandleEvent _handleEvent;
    };

    /// make_selectRectangle creates a SelectRectangle from a functor.
    template<typename Event, uint_fast16_t left, uint_fast16_t bottom, uint_fast16_t width, uint_fast16_t height, typename HandleEvent>
    SelectRectangle<Event, left, bottom, width, height, HandleEvent> make_selectRectangle(HandleEvent handleEvent) {
        return SelectRectangle<Event, left, bottom, width, height, HandleEvent>(std::forward<HandleEvent>(handleEvent));
    }
}
