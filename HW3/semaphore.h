typedef struct{int a; int b; int c; int d;} semaphore;
void down(semaphore* s);
void up(semaphore* s);
void createSem(semaphore* s, int value);
