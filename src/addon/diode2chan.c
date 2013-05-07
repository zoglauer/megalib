#include <stdio.h>

void main()
{

  const int X=0;
  const int Y=1;
  int channel;
  int Array[129][2];
  int x1chans[]={5,3,1,2,4};
  int x2chans[]={6,8,10,9,7};
  int count, yy, diodex, diodey;

  // Initialize Array
  for(channel=0;channel<129;channel++)
    {
      Array[channel][X]=0;
      Array[channel][Y]=0;
    }

  // Loop for reading in unconnected channels
  // They will be marked with -1 in Array[][]
  for(;channel!=-1;)
    {
      printf("Which channel is unconnected? (-1 to break) ");
      scanf("%d",&channel);
      if ( (channel<1) || (channel>128) ) continue;

      Array[channel][X]=-1;
      Array[channel][Y]=-1;
    }

  // Fill Array with X and Y coordinates
  count = 0;
  yy = 0;
  for(channel=1;channel<129;channel++)
    {
      if (Array[channel][X] != -1){
	if (count < 60){
	  Array[channel][X] = x1chans[(count%5)];
	  if ((count%5) == 0) yy++;
	  Array[channel][Y] = yy;
	}
	if (count == 60) yy = 13;
	if (count > 59){
	  Array[channel][X] = x2chans[(count%5)];
	  if ((count%5) == 0) yy--;
	  Array[channel][Y] = yy;
	}
	count++;
      }
    }

  /*
  // Test - did it fill up correctly?
  for (channel=0;channel<129;channel++)
    printf("%d ", Array[channel][X]);
  printf("\n");
  for (channel=0;channel<129;channel++)
    printf("%d ", Array[channel][Y]);
  printf("\n");
  */

  for(;;)
  {
    printf("Enter X of diode: ");
    scanf("%d",&diodex);
    printf("Enter Y of diode: ");
    scanf("%d",&diodey);

    if ( (diodex==-1) || (diodey==-1)) break;
    
    if ( (diodex<1) || (diodex>10) || (diodey<1) || (diodey>12) )
      { printf("What??\n");
      continue;
      }

    for(channel=1;channel<129;channel++)
      if((Array[channel][X] == diodex) && (Array[channel][Y] == diodey))
	printf("Diode (%d,%d) is channel %d\n", diodex, diodey, channel);
  }
  
}

	    
