/*
 * prng.h
 *
 * Created: 27.10.2015 18:44:16
 *  Author: washed
 */ 


#ifndef PRNG_H_
#define PRNG_H_


#define PRNG_SEED_W				1
#define PRNG_SEED_Z				2
#define BIAS_TABLE_FIELDS		2
#define BIAS_TABLE_PROB_INDEX	0
#define BIAS_TABLE_VALUE_INDEX	1

#define BIAS_SPEED_SIZE			256
#define BIAS_DIRECTION_SIZE		2



typedef struct prngTypeDef prngTypeDef;
typedef struct biasTableTypeDef biasTableTypeDef;


struct prngTypeDef
{	
	uint32_t			m_w;
	uint32_t			m_z;
};

struct biasTableTypeDef
{
	uint32_t			tableSize;
	const uint8_t*		table;
};


extern const uint8_t biasTableSpeedTable[BIAS_SPEED_SIZE][BIAS_TABLE_FIELDS];
extern const uint8_t biasTableDirectionTable[BIAS_DIRECTION_SIZE][BIAS_TABLE_FIELDS];

extern biasTableTypeDef biasTableSpeed;
extern biasTableTypeDef biasTableDirection;
extern biasTableTypeDef biasTableTime;

extern prngTypeDef prng1;

void init_random( prngTypeDef* prng_handle );
uint32_t get_random( prngTypeDef* prng_handle );
void seed_random( prngTypeDef* prng_handle, uint32_t seed );
uint8_t prng_is_seeded( prngTypeDef* prng_handle );
int32_t scale_and_offset( uint32_t scale, uint32_t offset, uint32_t input );
uint32_t lookup_value( biasTableTypeDef* biasTable_handle , uint32_t x );
uint32_t biased_random( prngTypeDef* prng_handle, biasTableTypeDef* biasTable_handle );


#endif /* PRNG_H_ */