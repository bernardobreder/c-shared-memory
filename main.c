#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

typedef unsigned char error_t;

struct sharedmemory_master_t {
	FILE* f;
	unsigned int size;
	unsigned char* bytes;
};

struct sharedmemory_slave_t {
	FILE* f;
	unsigned int size;
	unsigned char* bytes;
};

error_t sharedmemory_master_init(struct sharedmemory_master_t* self) {
	FILE* f = fopen("memory.data", "w+");
	if (!f) {
		return 1;
	}
	self->f = f;
	fseek(self->f, 0, SEEK_SET);
	fwrite(self->bytes, self->size, 1, self->f);
	fputc(0, self->f);
	fflush(self->f);
	return 0;
}

void sharedmemory_master_close(const struct sharedmemory_master_t* self) {
	fclose(self->f);
}

error_t sharedmemory_slave_init(struct sharedmemory_slave_t* self) {
	FILE* f = fopen("memory.data", "w+");
	if (!f) {
		return 1;
	}
	self->f = f;
	return 0;
}

void sharedmemory_slave_close(const struct sharedmemory_slave_t* self) {
	fclose(self->f);
}

error_t sharedmemory_can_write(const struct sharedmemory_master_t* self) {
	fseek(self->f, self->size, SEEK_SET);
	if (fgetc(self->f) == 1) {
		return 0;
	}
	return 1;
}

error_t sharedmemory_can_read(const struct sharedmemory_slave_t* self) {
	fseek(self->f, self->size, SEEK_SET);
	if (fgetc(self->f) == 0) {
		return 0;
	}
	return 1;
}

error_t sharedmemory_write(const struct sharedmemory_master_t* self) {
	fseek(self->f, 0, SEEK_SET);
	fwrite(self->bytes, self->size, 1, self->f);
	fputc(1, self->f);
	fflush(self->f);
	return 0;
}

error_t sharedmemory_read(const struct sharedmemory_slave_t* self) {
	fseek(self->f, 0, SEEK_SET);
	fread(self->bytes, self->size, 1, self->f);
	fputc(0, self->f);
	fflush(self->f);
	return 0;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("shared_memory (<master>|<slave>)\n");
		return 0;
	}
	int n, max = 8 * 1024;
	if (!strcmp(argv[1], "slave")) {
		clock_t time = clock();
		struct sharedmemory_slave_t slave;
		slave.size = 1024 * 768 * 3;
		slave.bytes = malloc(slave.size);
		sharedmemory_slave_init(&slave);
		for (n = 0; n < max; n++) {
			while (!sharedmemory_can_read(&slave)) {
				SDL_Delay(1);
			}
			sharedmemory_read(&slave);
		}
		clock_t end = clock();
		printf("Tempo: %lf miliseg/frame\n", (double) (end - time) / max);
	} else {
		struct sharedmemory_master_t master;
		master.size = 1024 * 768 * 3;
		master.bytes = malloc(master.size);
		sharedmemory_master_init(&master);
		for (n = 0; n < max; n++) {
			while (!sharedmemory_can_write(&master)) {
				SDL_Delay(1);
			}
			sharedmemory_write(&master);
		}
	}
	return 0;
}
