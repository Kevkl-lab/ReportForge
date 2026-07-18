#pragma once

#include <vector>
#include <algorithm>

namespace reportforge::core {

/**
 * @brief Generic interface for observers.
 */
template <typename T>
class Observer {
public:
    virtual ~Observer() = default;
    
    /**
     * @brief Called when the observed subject notifies changes.
     * @param event The event or entity that was updated.
     */
    virtual void onNotify(const T& event) = 0;
};

/**
 * @brief Generic subject that maintains list of observers and notifies them.
 */
template <typename T>
class Subject {
public:
    virtual ~Subject() = default;

    /**
     * @brief Register a new observer.
     */
    void addObserver(Observer<T>* observer) {
        if (observer && std::find(observers_.begin(), observers_.end(), observer) == observers_.end()) {
            observers_.push_back(observer);
        }
    }

    /**
     * @brief Unregister an observer.
     */
    void removeObserver(Observer<T>* observer) {
        observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
    }

    /**
     * @brief Notify all registered observers.
     */
    void notify(const T& event) {
        for (auto* observer : observers_) {
            if (observer) {
                observer->onNotify(event);
            }
        }
    }

private:
    std::vector<Observer<T>*> observers_;
};

} // namespace reportforge::core
