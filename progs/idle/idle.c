int test = 1;

int main(void)
{
  asm volatile("SVC #123");
  for(;;);
  return 0;
}
