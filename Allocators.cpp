#include "Allocators.h"
#include <climits>

int bestFit(int sizeInWords, void *list)
{
	int minLength = 65536;
	int offset = -1;
	uint16_t *holeList = static_cast<uint16_t *>(list);
	uint16_t holelength = *holeList++;
	for(uint16_t i = 1; i < (holelength) * 2; i += 2)
	{
		if(holeList[i] >= sizeInWords && holeList[i] < minLength)
		{
			offset = holeList[i - 1];
			minLength = holeList[i];
		}
	}
	return offset;
}

int worstFit(int sizeInWords, void *list)
{
	int maxLen = -1;
	int offset = -1;

	uint16_t *holeList = static_cast<uint16_t *>(list);
	uint16_t holelength = *holeList++;

	for(uint16_t i = 1; i < (holelength) * 2; i += 2)
	{
		if(holeList[i] >= sizeInWords && holeList[i] > maxLen)
		{
			offset = holeList[i - 1];
			maxLen = holeList[i];
		}
	}

	return offset;
}
