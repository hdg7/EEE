#include "BubbleType.h"

const char* BubbleType::FOLDER_NAME = "Bubbles";
const char* BubbleType::TOTALTYPES_NAME = "total";
const char* BubbleType::BUBBLE_NAME = "bubble";
const char* BubbleType::NUMBERBUBBLES_NAME = "Number";
const char* BubbleType::MEANBUBBLE_NAME = "Median";
const char* BubbleType::SDBUBBLE_NAME = "Deviation";
const char* BubbleType::SIZEBUBBLE_NAME = "Size";
const char* BubbleType::DENSITYBUBBLE_NAME = "Density";

BubbleType::BubbleType(int number)
{
	char line[LINE_L];
	char aux[LINE_L];
	bubbleNumber = number;
//Read configuration file

	sprintf(aux,"%s%d",BUBBLE_NAME,number);

        readConfigurationS(aux,NUMBERBUBBLES_NAME,line,LINE_L);
	numberBubbles=atoi(line);
        readConfigurationS(aux,MEANBUBBLE_NAME,line,LINE_L);
	meanBubbles =atof(line);
        readConfigurationS(aux,SDBUBBLE_NAME,line,LINE_L);
	sdBubbles =atof(line);
        readConfigurationS(aux,SIZEBUBBLE_NAME,line,LINE_L);
	sizeBubbles =atof(line);
        readConfigurationS(aux,DENSITYBUBBLE_NAME,line,LINE_L);
	densityBubbles = atof(line);

}

double BubbleType::randn (double mu, double sigma)
{
  double U1, U2, W, mult;
  static double X1, X2;
  static int call = 0;

  if (call == 1)
    {
      call = !call;
      return (mu + sigma * (double) X2);
    }

  do
    {
      U1 = -1 + ((double) rand () / RAND_MAX) * 2;
      U2 = -1 + ((double) rand () / RAND_MAX) * 2;
      W = pow (U1, 2) + pow (U2, 2);
    }
  while (W >= 1 || W == 0);

  mult = sqrt ((-2 * log (W)) / W);
  X1 = U1 * mult;
  X2 = U2 * mult;

  call = !call;

  return (mu + sigma * (double) X1);
}

int BubbleType::InitializeBubbles(unsigned compressArea)
{
	int i;
	unsigned int position;

	positions = (int *)malloc(sizeof(int)*numberBubbles);
	for(i=0; i < numberBubbles; i++)
        {
       		position = randn(compressArea*meanBubbles,
                	    compressArea*sdBubbles);
                if(position > 0 && position < compressArea)
                	positions[i] = position;
                else
                       	positions[i] = -1;
        }

	return 0;
}

int BubbleType::getBubbleSize()
{
	return (int) (sizeBubbles*1000.0);
}
int BubbleType::getBubblePosition(int i)
{
	return positions[i];
}
double BubbleType::getBubbleDensity()
{
	return densityBubbles;
}
int BubbleType::getBubbleNumber()
{
	return numberBubbles;
}
char *BubbleType::getBubbleSections(int *blocks)
{
	int i=0;
	int jumps[] = {8,7,4,6,4,7,8,2};
	char *output;
	char aux[400];

	output = (char *) malloc(1000*sizeof(char));
	output[0] = 0;

	for(i =0; i < 8; i++)
	{
		sprintf(aux,"BUBBLES_%d_V%d,BUBBLES_%d_%d_V%d,",i+1,blocks[i],i+1,jumps[i],blocks[jumps[i]-1]);
		strcat(output,aux);
	}
      	strcat(output,"BUBBLES_9");
//	printf("%s\n",output);
/*    static char bubbleNames[] =
        "BUBBLES_INI_V1,+80CXXXX,BUBBLES_1_8_V1,BUBBLES_02_V1,+80CXXXX,BUBBLES_2_7_V1,"
	"BUBBLES_03_V1,+80CXXXX,BUBBLES_3_4_V1,BUBBLES_04_V1,+80CXXXX,BUBBLES_4_6_V1,"
        "BUBBLES_05_V1,+80CXXXX,BUBBLES_5_4_V1,BUBBLES_06_V1,+80CXXXX,BUBBLES_6_7_V1,"
	"BUBBLES_07_V1,+80CXXXX,BUBBLES_7_8_V1,BUBBLES_08_V1,+80CXXXX,BUBBLES_8_2_V1,"
        "BUBBLES_09_V1";
*//*    static char bubbleNames_V1[] =
        "BUBBLES_01_V1,BUBBLES_1_8_V1,BUBBLES_02_V1,BUBBLES_2_7_V1,"
	"BUBBLES_03_V1,BUBBLES_3_4_V1,BUBBLES_04_V1,BUBBLES_4_6_V1,"
        "BUBBLES_05_V1,BUBBLES_5_4_V1,BUBBLES_06_V1,BUBBLES_6_7_V1,"
	"BUBBLES_07_V1,BUBBLES_7_8_V1,BUBBLES_08_V1,BUBBLES_8_2_V1,"
        "BUBBLES_09";
 static char bubbleNames_V2[] =
        "BUBBLES_01_V2,BUBBLES_1_8_V2,BUBBLES_02_V2,BUBBLES_2_7_V2,"
        "BUBBLES_03_V2,BUBBLES_3_4_V2,BUBBLES_04_V2,BUBBLES_4_6_V2,"
       	"BUBBLES_05_V2,BUBBLES_5_4_V2,BUBBLES_06_V2,BUBBLES_6_7_V2,"
        "BUBBLES_07_V2,BUBBLES_7_8_V2,BUBBLES_08_V2,BUBBLES_8_2_V2,"
        "BUBBLES_09";
*//* static char bubbleNames_V3[] =
        "BUBBLES_01_V3,BUBBLES_1_8_V3,BUBBLES_02_V3,BUBBLES_2_7_V3,"
        "BUBBLES_03_V3,BUBBLES_3_4_V3,BUBBLES_04_V3,BUBBLES_4_6_V3,"
       	"BUBBLES_05_V3,BUBBLES_5_4_V3,BUBBLES_06_V3,BUBBLES_6_7_V3,"
        "BUBBLES_07_V3,BUBBLES_7_8_V3,BUBBLES_08_V3,BUBBLES_8_2_V3,"
        "BUBBLES_09";
*/ /*static char bubbleNames_V4[] =
        "BUBBLES_01_V4,BUBBLES_1_8_V4,BUBBLES_02_V4,BUBBLES_2_7_V4,"
        "BUBBLES_03_V4,BUBBLES_3_4_V4,BUBBLES_04_V4,BUBBLES_4_6_V4,"
       	"BUBBLES_05_V4,BUBBLES_5_4_V4,BUBBLES_06_V4,BUBBLES_6_7_V4,"
        "BUBBLES_07_V4,BUBBLES_7_8_V4,BUBBLES_08_V4,BUBBLES_8_2_V4,"
        "BUBBLES_09";
*//* static char bubbleNames_V3[] =
       	"BUBBLES_01_V3,BUBBLES_1_8_V3,BUBBLES_02_V4,BUBBLES_2_7_V3,"
       	"BUBBLES_03_V3,BUBBLES_3_4_V3,BUBBLES_04_V3,BUBBLES_4_6_V3,"
       	"BUBBLES_05_V3,BUBBLES_5_4_V3,BUBBLES_06_V3,BUBBLES_6_7_V3,"
       	"BUBBLES_07_V3,BUBBLES_7_8_V3,BUBBLES_08_V3,BUBBLES_8_2_V4,"
       	"BUBBLES_09";
*/	return output;
}
