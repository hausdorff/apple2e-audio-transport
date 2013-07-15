#include <stdio.h>
#include <stdlib.h>

#define LEADER_LENGTH 300
#define HZ 44100

double onePeriod(int frequency, double inRemainder)
{
    int numBytes = HZ / (2 * frequency);
    double remainder = inRemainder + (double)(((double)(HZ / (double)(2 * frequency))-numBytes)/20);
    unsigned char value = 1;
    int i;
    for (i = 0; i < numBytes; i++) {
            putchar(value);
    }
    if (remainder > 1.0) {
        putchar(value);
    }
    value = -1;
    for (i = 0; i < numBytes; i++) {
        putchar(value);
    }
    if (remainder > 1.0) {
        putchar(value);
        remainder -= 1.0;
    }
    return remainder;
}

void leaderTone(int duration)
{
    for (int i = 0; i < duration; i++) {
        onePeriod(770, 0.0);
    }
}

void silence()
{
    for (int i = 0; i < 15000; i++) {
        putchar(0);
    }
}

double encodeByte(unsigned char data, double inRemainder)
{
    double remainder = inRemainder;
    for (int j = 0; j < 8; j++) {
        if ((data & 0x00000080) == 0x00000080) {
            // It's a 1
            remainder = onePeriod(1000, remainder);
            //Log.print(false,"1");
        }
        else {
            // It's a 0
            remainder = onePeriod(2000, remainder);
            //Log.print(false,"0");
        }
        data = (unsigned char) ((data << 1) & 0x000000FFL);
        //Log.println(false,"BytesToWav.encodeByte() remainder after bit: "+remainder);
    }
    //Log.println(false,"BytesToWav.encodeByte() returning remainder: "+remainder);
    return remainder;
}

unsigned char encodeData(unsigned char *myFile, int length,
                         unsigned char checksum)
{
    int i;
    double remainder = 0.0;
    for (i = 0; i < length; i++) {
        checksum = (unsigned char) (myFile[i] ^ checksum);
        remainder = encodeByte(myFile[i], remainder);
    }
    return checksum;
}

int main(int argc, char *argv[])
{
    unsigned char *myFile;

    if (argc != 2) {
        printf("usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    rewind(fp);
    myFile = (unsigned char *)malloc(length);
    if (fread(myFile, 1, length, fp) != length) {
        perror("fread");
        return 1;
    }

    fprintf(stderr,"Length: %d\n", length);
    fprintf(stderr, "Load at: %.4x..%.4x\n", 0x800,0x800+length-1);
    fprintf(stderr,"Press enter when ready...");
    getchar();

    unsigned char checksum = -1;
    leaderTone(LEADER_LENGTH);
    onePeriod(2200, 0.0);
    checksum = encodeData(myFile, length, checksum);
    encodeByte(checksum, 0.0);
    onePeriod(200, 0.0);
    silence();
}
