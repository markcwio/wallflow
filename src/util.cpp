#include "util.h"

bool StringVectorsMatch(const std::vector<std::string>& vector1, const std::vector<std::string>& vector2)
{
    if (vector1.size() != vector2.size()) {
        return false;
    }

    std::vector<std::string> sortedVector1 = vector1;
    std::vector<std::string> sortedVector2 = vector2;

    std::sort(sortedVector1.begin(), sortedVector1.end());
    std::sort(sortedVector2.begin(), sortedVector2.end());

    for (size_t i = 0; i < sortedVector1.size(); ++i) {
        if (sortedVector1[i] != sortedVector2[i]) {
            return false;
        }
    }

    return true;
}