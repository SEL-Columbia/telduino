#include "statistics.h"
/** 
  From http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
  */
void variancef(float *value, float *mean, float *variance, int32_t *nOld)
{
	 
	float m2;
    float delta = *value - *mean;
    *nOld = *nOld + 1;
	*mean = *mean + delta/ *nOld;
	m2 = m2 + delta*(*value - *mean);
 
    //variance_n = M2/n;
    *variance = m2/(*nOld-1);
}

