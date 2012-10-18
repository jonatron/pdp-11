#include <stdio.h>

int bigEndian;

int isBigEndian() {
	int no = 1;
	char *chk = (char *)&no;

	if (chk[0] == 1) {
		return 0;
	}
	else {
		return 1;
	}
}

int main(int argc, char ** argv) {
	int i;
	FILE *f = fopen("test.dsk", "wb");
	bigEndian = isBigEndian();
	unsigned int s = 256 * 203 * 12;
	//printf("%d\n", s*2);
	unsigned short buffer[1247232];
	int se = 1;
	int j;
	i = 0;
	buffer[0] = 0;
	for (j = 1; j < s*2; j++) {
		if ((j % 256) == 0) {
			buffer[j] = se;
			se++;
			i = 0;
			//continue;
		}
		else {
			buffer[j] = i;
			i++;
		}
		if (bigEndian) {
			unsigned char high, low;
			high = buffer[j] & 255;
			low = (buffer[j] >> 8) & 255;
			buffer[j] = (high << 8) + low;
		}
		//printf("byte %d done, buffr %d\n", j, buffer[j]);

	}
	//printf("Setup complete, %d done\n", j);

	//printf("conversion complete");
	fwrite(buffer, sizeof(unsigned short), s*2, f);
	fclose(f);
	return 0;
}
