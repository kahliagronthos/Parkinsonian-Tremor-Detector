/**
 * @brief This class represents the moving average of a window of data.
 *
 * @tparam T The type of elements in the array.
 * @tparam ARRAY_LEN The length of the array.
 */
template <class T, uint8_t ARRAY_LEN>
class MovingAverage {
private:
    const uint8_t WINDOW_SIZE = ARRAY_LEN;
    T window[ARRAY_LEN] = {0};
    uint8_t size = 0;
    uint8_t index = 0; 

    T sum = 0;

public:
    /**
     * Updates a sliding window with a new value.
     *
     * @param new_value The new value to be added to the window.
     *
     * @returns None
     */
    void update(T new_value) {
        // Fill up window first, otherwise replace value
        if (size < WINDOW_SIZE)
            size += 1;
        else
            sum -= window[index];
        
        sum += new_value;
        window[index] = new_value;

        // Set index to next data slot, wrapping around to oldest
        if (index < WINDOW_SIZE)
            index++;
        else
            index = 0;
    }

    /**
     * Calculates the average of the current window of data
     *
     * @returns (type T) The average of the data.
     */
    T getAverage() {
        if (size <= 0)
            return 0;
        else {
            T summation = 0;
            for (int i = 0; i < size; i++){
                summation += window[i];
            }
            return static_cast<T>(summation / static_cast<float>(size));
        }
            
    }

    void clear() {
        for (int i = 0; i < WINDOW_SIZE; i++) {
            window[i] = 0;
        }
        size = 0;
    }
};

// Pre-load common data types
template class MovingAverage<double, 10>;
template class MovingAverage<float, 10>;
template class MovingAverage<int, 10>;