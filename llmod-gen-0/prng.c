/*
 * prng.c
 *
 * Created: 12.09.2015 16:41:35
 *  Author: washed
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "prng.h"

const uint8_t biasTableSpeedTable[BIAS_SPEED_SIZE][BIAS_TABLE_FIELDS] PROGMEM =
{
	{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0},
	{8, 0}, {9, 0}, {10, 0}, {11, 0}, {12, 0}, {13, 0}, {14, 0}, {15, 0},
	{16, 0}, {17, 0}, {18, 0}, {19, 0}, {20, 0}, {21, 0}, {22, 0}, {23, 0},
	{24, 0}, {25, 0}, {26, 0}, {27, 0}, {28, 0}, {29, 0}, {30, 0}, {31, 0},
	{32, 0}, {33, 0}, {34, 0}, {35, 0}, {36, 0}, {37, 0}, {38, 0}, {39, 0},
	{40, 0}, {41, 0}, {42, 0}, {43, 0}, {44, 0}, {45, 0}, {46, 0}, {47, 0},
	{48, 0}, {49, 0}, {50, 0}, {51, 0}, {52, 0}, {53, 0}, {54, 0}, {55, 0},
	{56, 0}, {57, 0}, {58, 0}, {59, 0}, {60, 0}, {61, 0}, {62, 0}, {63, 0},
	{64, 0}, {65, 0}, {66, 0}, {67, 0}, {68, 0}, {69, 0}, {70, 0}, {71, 0},
	{72, 0}, {73, 0}, {74, 0}, {75, 0}, {76, 0}, {77, 0}, {78, 0}, {79, 0},
	{80, 0}, {81, 0}, {82, 0}, {83, 0}, {84, 0}, {85, 0}, {86, 0}, {87, 0},
	{88, 0}, {89, 0}, {90, 0}, {91, 0}, {92, 0}, {93, 0}, {94, 0}, {95, 0},
	{96, 0}, {97, 0}, {98, 0}, {99, 0}, {100, 0}, {101, 0}, {102, 0}, {103, 0},
	{104, 0}, {105, 0}, {106, 0}, {107, 0}, {108, 0}, {109, 0}, {110, 0}, {111, 0},
	{112, 0}, {113, 0}, {114, 0}, {115, 0}, {116, 0}, {117, 0}, {118, 0}, {119, 0},
	{120, 0}, {121, 0}, {122, 0}, {123, 0}, {124, 0}, {125, 0}, {126, 0}, {127, 0},
	{128, 0}, {129, 0}, {130, 0}, {131, 0}, {132, 0}, {133, 0}, {134, 0}, {135, 0},
	{136, 0}, {137, 0}, {138, 0}, {139, 0}, {140, 0}, {141, 0}, {142, 0}, {143, 0},
	{144, 0}, {145, 0}, {146, 0}, {147, 0}, {148, 0}, {149, 0}, {150, 0}, {151, 0},
	{152, 0}, {153, 0}, {154, 0}, {155, 0}, {156, 0}, {157, 0}, {158, 0}, {159, 0},
	{160, 0}, {161, 0}, {162, 0}, {163, 0}, {164, 0}, {165, 0}, {166, 0}, {167, 0},
	{168, 0}, {169, 0}, {170, 0}, {171, 0}, {172, 0}, {173, 0}, {174, 0}, {175, 0},
	{176, 0}, {177, 0}, {178, 0}, {179, 0}, {180, 0}, {181, 0}, {182, 0}, {183, 0},
	{184, 0}, {185, 0}, {186, 0}, {187, 0}, {188, 0}, {189, 0}, {190, 0}, {191, 0},
	{192, 0}, {193, 0}, {194, 0}, {195, 0}, {196, 0}, {197, 0}, {198, 0}, {199, 0},
	{200, 0}, {201, 0}, {202, 0}, {203, 0}, {204, 0}, {205, 0}, {206, 0}, {207, 0},
	{208, 0}, {209, 0}, {210, 0}, {211, 0}, {212, 0}, {213, 0}, {214, 0}, {215, 0},
	{216, 0}, {217, 0}, {218, 0}, {219, 0}, {220, 0}, {221, 0}, {222, 0}, {223, 0},
	{224, 0}, {225, 0}, {226, 0}, {227, 0}, {228, 0}, {229, 0}, {230, 0}, {231, 0},
	{232, 0}, {233, 0}, {234, 0}, {235, 0}, {236, 0}, {237, 0}, {238, 0}, {239, 0},
	{240, 0}, {241, 0}, {242, 0}, {243, 0}, {244, 0}, {245, 0}, {246, 0}, {247, 0},
	{248, 0}, {249, 0}, {250, 0}, {251, 0}, {252, 0}, {253, 0}, {254, 0}, {255, 0}
	
};

const uint8_t biasTableDirectionTable[BIAS_DIRECTION_SIZE][BIAS_TABLE_FIELDS] PROGMEM =
{
	{0,1},{0,4}
};

prngTypeDef prng1;
biasTableTypeDef biasTableSpeed;
biasTableTypeDef biasTableDirection;
biasTableTypeDef biasTableTime;

void init_random( prngTypeDef* prng_handle )
{
	prng_handle->m_w = PRNG_SEED_W;
	prng_handle->m_z = PRNG_SEED_Z;
	
	biasTableSpeed.tableSize = BIAS_SPEED_SIZE;
	biasTableSpeed.table = (const uint8_t*)(*biasTableSpeedTable)[BIAS_TABLE_FIELDS];
	
	biasTableDirection.tableSize = BIAS_DIRECTION_SIZE;
	biasTableDirection.table = (*biasTableDirectionTable)[BIAS_TABLE_FIELDS];
	
	//TODO: Init the other tables here as well!
}

uint32_t get_random( prngTypeDef* prng_handle )
{
	prng_handle->m_z = 36969L * (prng_handle->m_z & 65535L) + (prng_handle->m_z >> 16);
	prng_handle->m_w = 18000L * (prng_handle->m_w & 65535L) + (prng_handle->m_w >> 16);
	return (prng_handle->m_z << 16) + prng_handle->m_w;  /* 32-bit result */
}

void seed_random( prngTypeDef* prng_handle, uint32_t seed )
{
	for ( uint32_t i = seed; i != 0; i-- )
	get_random( prng_handle );
	prng_handle->m_z = get_random( prng_handle );
	for ( uint32_t i = seed; i != 0; i-- )
	get_random( prng_handle );
	prng_handle->m_w = get_random( prng_handle );
}

uint8_t prng_is_seeded( prngTypeDef* prng_handle )
{
	if ( (prng_handle->m_w == PRNG_SEED_W) || (prng_handle->m_z == PRNG_SEED_Z) )
	return 0;
	else
	return 1;
}

int32_t scale_and_offset( uint32_t scale, uint32_t offset, uint32_t input )
{
	return( (input % scale) + offset);
}

/*
uint32_t lookup_value( biasTableTypeDef* biasTable_handle , uint32_t x )
{
	uint32_t cumulative_weight = 0;
	for (uint32_t i = 0; i < biasTable_handle->tableSize; i++)
	{
		cumulative_weight += (*biasTable_handle->table)[i][BIAS_TABLE_PROB_INDEX];
		if (x < cumulative_weight)
			return biasTable_handle->table[i][BIAS_TABLE_VALUE_INDEX];
	}
	return 0;
}


uint32_t biased_random( prngTypeDef* prng_handle, biasTableTypeDef* biasTable_handle )
{
	uint32_t sum_of_weights = 0;
	for (int i = 0; i < biasTable_handle->tableSize; i++)
		sum_of_weights += biasTable_handle->table[i][BIAS_TABLE_PROB_INDEX];

	return lookup_value( biasTable_handle, scale_and_offset(sum_of_weights, 0, get_random( prng_handle )));
}
*/