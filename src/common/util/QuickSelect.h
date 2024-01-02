// Copyright (c) 2024 Clarisma / GeoDesk contributors
// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <algorithm> // For std::swap
#include <cmath>
#include <functional> // For std::function


/*
 * This class is based on QuickSelect (https://github.com/mourner/quickselect)
 * by Volodymyr Agafonkin. The original work is licensed as follows:
 *
 * ISC License
 *
 * Copyright (c) 2018, Volodymyr Agafonkin
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice
 * and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 *
 * (https://github.com/mourner/quickselect/blob/master/LICENSE)
 */

template <typename T>
class QuickSelect
{
private:
    static void swap(T* arr, int i, int j)
    {
        std::swap(arr[i], arr[j]);
    }

    static void quickselectStep(T* arr, int k, int left, int right, const std::function<int(const T&, const T&)>& compare)
    {
        while (right > left)
        {
            if (right - left > 600)
            {
                double n = right - left + 1;
                double m = k - left + 1;
                double z = log(n);
                double s = 0.5 * exp(2 * z / 3);
                double sd = 0.5 * sqrt(z * s * (n - s) / n) * (m - n / 2 < 0 ? -1 : 1);
                int newLeft = std::max(left, static_cast<int>(floor(k - m * s / n + sd)));
                int newRight = std::min(right, static_cast<int>(floor(k + (n - m) * s / n + sd)));
                quickselectStep(arr, k, newLeft, newRight, compare);
            }

            const T& t = arr[k];
            int i = left;
            int j = right;

            swap(arr, left, k);
            if (compare(arr[right], t) > 0) swap(arr, left, right);

            while (i < j)
            {
                swap(arr, i, j);
                i++;
                j--;
                while (compare(arr[i], t) < 0) i++;
                while (compare(arr[j], t) > 0) j--;
            }

            if (compare(arr[left], t) == 0)
            {
                swap(arr, left, j);
            }
            else
            {
                j++;
                swap(arr, j, right);
            }
            if (j <= k) left = j + 1;
            if (k <= j) right = j - 1;
        }
    }

public:
    static void multiSelect(T* arr, int left, int right, int n, const std::function<int(const T&, const T&)>& compare)
    {
        std::stack<int> stack;
        stack.push(left);
        stack.push(right);

        while (!stack.empty())
        {
            right = stack.top(); stack.pop();
            left = stack.top(); stack.pop();

            if (right - left <= n) continue;

            int mid = left + static_cast<int>(std::ceil(static_cast<double>(right - left) / n / 2)) * n;
            quickselect(arr, mid, left, right, compare);

            stack.push(left);
            stack.push(mid);
            stack.push(mid);
            stack.push(right);
        }
    }

    static void quickselect(T* arr, size_t size, int k, int left, int right, const std::function<int(const T&, const T&)>& compare)
    {
        quickselectStep(arr, k, left, right != 0 ? right : static_cast<int>(size - 1), compare);
    }
};
