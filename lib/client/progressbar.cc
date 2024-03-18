#include "session_client.h"

void Session::ProgressbarDown()
{
   const char bar = '=';          // 프로그레스바 문자
   const char blank = ' ';        // 비어있는 프로그레스바 문자
   const int LEN = 20;            // 프로그레스바 길이
   const int MAX = 100;           // 진행작업 최대값
   int count = 0;                 // 현재 진행된 작업
   int i;                         // 반복문 전용 변수
   float tick = (float)100 / LEN; // 몇 %마다 프로그레스바 추가할지 계산
   printf("%0.2f%% 마다 bar 1개 출력\n\n", tick);
   int bar_count; // 프로그레스바 갯수 저장 변수
   float percent; // 퍼센트 저장 변수
   while (count <= MAX)
   {
      printf("\r%d/%d [", count, MAX);    // 진행 상태 출력
                                          //   fflush(stdout);
      percent = (float)count / MAX * 100; // 퍼센트 계산
      bar_count = percent / tick;         // 프로그레스바 갯수 계산
      for (i = 0; i < LEN; i++)
      { // LEN길이의 프로그레스바 출력
         if (bar_count > i)
         { // 프로그레스바 길이보다 i가 작으면
            printf("%c", bar);
         }
         else
         { // i가 더 커지면
            printf("%c", blank);
         }
      }
      printf("] %0.2f%%", percent); // 퍼센트 출력
      double throughput_down = ((total_downloaddata * 8) / 1000000.0d) / time_;
      if (count == MAX - 1)
      {
         while (delay_down != number_)
         {
         }
         std::cout << ": download throughput = " << throughput_down << "Mbps";
      }
      else
         std::cout << ": download throughput = " << throughput_down << "Mbps";
      fflush(stdout);
      count++;              // 카운트 1증가
      usleep(time_ * 10000); // spended time per 1/100
   }
   printf(" done!\n\n");
   // system("pause"); // 프로그램 종료 전 일시정지
}
void Session::ProgressbarUp()
{
   const char bar = '=';          // 프로그레스바 문자
   const char blank = ' ';        // 비어있는 프로그레스바 문자
   const int LEN = 20;            // 프로그레스바 길이
   const int MAX = 100;           // 진행작업 최대값
   int count = 0;                 // 현재 진행된 작업
   int i;                         // 반복문 전용 변수
   float tick = (float)100 / LEN; // 몇 %마다 프로그레스바 추가할지 계산
   // printf("%0.2f%% 마다 bar 1개 출력\n\n", tick);
   int bar_count; // 프로그레스바 갯수 저장 변수
   float percent; // 퍼센트 저장 변수
   while (count <= MAX)
   {
      printf("\r%d/%d [", count, MAX);    // 진행 상태 출력
                                          //   fflush(stdout);
      percent = (float)count / MAX * 100; // 퍼센트 계산
      bar_count = percent / tick;         // 프로그레스바 갯수 계산
      for (i = 0; i < LEN; i++)
      { // LEN길이의 프로그레스바 출력
         if (bar_count > i)
         { // 프로그레스바 길이보다 i가 작으면
            printf("%c", bar);
         }
         else
         { // i가 더 커지면
            printf("%c", blank);
         }
      }
      printf("] %0.2f%% ", percent); // 퍼센트 출력
      double throughput_up = ((total_uploaddata * 8) / 1000000.0d) / time_;

      if (count == MAX - 1)
      {
         while (delay_up != number_)
         {
         }
         std::cout << ": upload throughput =  " << throughput_up << "Mbps";
      }
      else
         std::cout << ": upload throughput =  " << throughput_up << "Mbps";
      fflush(stdout);
      count++;              // 카운트 1증가
      usleep(time_ * 10000); // // spended time per 1/100
   }
   printf(" done!\n\n");
   // system("pause"); // 프로그램 종료 전 일시정지
}