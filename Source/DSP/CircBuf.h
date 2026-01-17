#pragma once

#include <assert.h>
#include <vector>

/** A lightweight CircBuf which returns samples with a value of zero
 *  when there are not enough samples ready. Not thread safe !
 */
template<typename SampleType>
class CircBuf
{

public:

    CircBuf() = default;

    CircBuf(int size) { setSize(size); }

    /** Set the size of the fifo.
     *  This also clears the fifo's internal buffer.
     *
     * @param size          The number of samples the fifo can hold.
     */
    void setSize(int size)
    {
        data.resize(size, static_cast<SampleType>(0.0));
        std::fill(data.begin(), data.end(), static_cast<SampleType>(0.0));

        writeIndex = 0;
    }

    /** Add a sample to the buffer.\n\n It's the callers responsibilty to
     *  advance the write index by calling incrementWriteIndex().
     *
     * @param sample        The sample value to add.
     *
     * @see incrementWriteIndex
     */
    void write(SampleType sample)
    {
        // Initialize the fifo first !
        assert(data.size() > 0);

        data[writeIndex] = sample;
    }

    /** Read a sample from the buffer.
     *
     * @param index         The distance from the current write index.
     */
    SampleType read(int index)
    {
        assert(index < data.size());

        int idx = writeIndex - index;
        if (idx < 0) idx += data.size();

        return data[idx];
    }

    /** Clears the buffer.*/
    void clear()
    {
        std::fill(data.begin(), data.end(), static_cast<SampleType>(0.0));
        writeIndex = 0;
    }

    void incrementWriteIndex()
    {
        if (++writeIndex >= data.size())
            writeIndex = 0;
    }

    int getWriteIndex() const { return writeIndex; }

private:

    std::vector<SampleType> data;
    int writeIndex { 0 };
};