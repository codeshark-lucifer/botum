#pragma once
#include <engine/utils.h>

#include <string>
#include <algorithm>

struct GapBufferBytes
{
    Array<c32> buf;

    size_t start = 0; // gap start
    size_t end = 0;   // gap end (exclusive)
    size_t cursor() const { return start; }
    void set_cursor(size_t pos) { move(pos); }
    // ---------------------------------------------------------
    // Constructor
    // ---------------------------------------------------------
    GapBufferBytes(size_t reserve = 1024)
    {
        buf.resize(reserve);
        start = 0;
        end = buf.size(); // entire buffer is gap
    }

    // ---------------------------------------------------------
    // Basic properties
    // ---------------------------------------------------------

    size_t capacity() const { return buf.size(); }

    // size of gap
    size_t gap_size() const { return end - start; }

    // number of actual characters stored
    size_t length() const { return buf.size() - gap_size(); }

    // ---------------------------------------------------------
    // Convert logical index -> internal index
    // ---------------------------------------------------------
    size_t internal(size_t pos) const
    {
        Assert(pos <= length(), "pos > length()");
        if (pos < start)
            return pos;
        else
            return pos + gap_size();
    }

    // ---------------------------------------------------------
    // Ensure gap has at least `min` free slots
    // ---------------------------------------------------------
    void ensure(size_t min = 1)
    {
        if (gap_size() >= min)
            return;

        size_t oldCap = buf.size();
        size_t grow = std::max(min, oldCap);
        size_t newCap = oldCap + grow;

        Array<c32> newBuf;
        newBuf.resize(newCap);

        // copy before gap
        std::copy(buf.begin(), buf.begin() + start,
                  newBuf.begin());

        // copy after gap
        size_t afterCount = oldCap - end;
        std::copy(buf.begin() + end, buf.end(),
                  newBuf.begin() + (newCap - afterCount));

        end = newCap - afterCount;
        buf.swap(newBuf);
    }

    // ---------------------------------------------------------
    // Move gap to logical position
    // ---------------------------------------------------------
    void move(size_t pos)
    {
        Assert(pos <= length(), "pos > length()");

        if (pos == start)
            return;

        if (pos < start)
        {
            // move gap left
            size_t count = start - pos;

            for (size_t i = 0; i < count; ++i)
                buf[end - 1 - i] = buf[start - 1 - i];

            start = pos;
            end -= count;
        }
        else
        {
            // move gap right
            size_t count = pos - start;

            for (size_t i = 0; i < count; ++i)
                buf[start + i] = buf[end + i];

            start += count;
            end += count;
        }
    }

    // ---------------------------------------------------------
    // Insert character at cursor (gap)
    // ---------------------------------------------------------
    void insert(c32 c)
    {
        ensure(1);
        buf[start++] = c;
    }

    // ---------------------------------------------------------
    // Backspace (delete before cursor)
    // ---------------------------------------------------------
    bool backward()
    {
        if (start == 0)
            return false;

        start--;
        return true;
    }

    // ---------------------------------------------------------
    // Delete key (delete after cursor)
    // ---------------------------------------------------------
    bool forward()
    {
        if (end == buf.size())
            return false;

        end++;
        return true;
    }

    // ---------------------------------------------------------
    // Get character at logical position
    // ---------------------------------------------------------
    c32 get(size_t pos) const
    {
        Assert(pos < length(), "pos >= length()");
        return buf[internal(pos)];
    }

    // ---------------------------------------------------------
    // Clear entire buffer
    // ---------------------------------------------------------
    void clear()
    {
        start = 0;
        end = buf.size();
    }

    // ---------------------------------------------------------
    // Convert to UTF-8 string (for rendering/saving)
    // ---------------------------------------------------------
    std::string to_string() const
    {
        std::string out;
        out.reserve(length());

        for (size_t i = 0; i < length(); ++i)
        {
            c32 c = get(i);

            // Basic UTF-32 → UTF-8 encoding
            if (c <= 0x7F)
                out.push_back((char)c);
            else if (c <= 0x7FF)
            {
                out.push_back((char)(0xC0 | (c >> 6)));
                out.push_back((char)(0x80 | (c & 0x3F)));
            }
            else if (c <= 0xFFFF)
            {
                out.push_back((char)(0xE0 | (c >> 12)));
                out.push_back((char)(0x80 | ((c >> 6) & 0x3F)));
                out.push_back((char)(0x80 | (c & 0x3F)));
            }
            else
            {
                out.push_back((char)(0xF0 | (c >> 18)));
                out.push_back((char)(0x80 | ((c >> 12) & 0x3F)));
                out.push_back((char)(0x80 | ((c >> 6) & 0x3F)));
                out.push_back((char)(0x80 | (c & 0x3F)));
            }
        }

        return out;
    }

    // ---------------------------------------------------------
    // Set buffer from ASCII/UTF-8 string (simple version)
    // NOTE: This treats input as ASCII only.
    // ---------------------------------------------------------
    void set(const std::string &s)
    {
        size_t cap = std::max(s.size() + 16ull, 64ull);

        buf.assign(cap, 0);

        start = s.size();
        end = cap;

        for (size_t i = 0; i < s.size(); ++i)
            buf[i] = (unsigned char)s[i];
    }
};
