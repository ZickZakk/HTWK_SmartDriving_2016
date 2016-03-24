/**
 * @author pbachmann
 */

#include "StringUtils.h"

bool StringUtils::StringContains(string text, string find)
{
    return text.find(find) != string::npos;
}