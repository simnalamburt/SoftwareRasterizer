Multi-Threaded Software Rasterizer
==================
멀티스레디드 소프트웨어 래스터라이저

프로젝트 1
- 김지현
- 이효원

----
#### 컴파일 하기 전 주의사항
비주얼 스튜디오로 프로그램을 실행하였을때, Intel TBB dll 파일들을 찾지 못한다면 아래의 절차를 수행하면 된다
 1. 'Executable' 프로젝트 설정 (ALT+F7)
 2. 'Coinfiguration Properties' -> 'Debugging' -> 'Environment' -> 우클릭 -> 'Edit'
 3. 다음 문장 추가
    
        PATH=%PATH%;$(SolutionDir)Intel TBB\bin\ia32\vc11;$(SolutionDir)FBX\lib\x86\$(Configuration)
