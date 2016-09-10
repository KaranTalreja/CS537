Benchmarks
Run command: Command: ./bin/varsort -i gen_case/out/outfile.100.32 -o out

Base Case: "return 0" only in main : 99,488

Read Only file: 174,088 (+74600 +74.98%)

Read and Create OM : 239,752 (Upgraded)

Changed OM from list to Array : 197,545 (Upgraded)

Changed OM for Array of objects to Array of pointers : 217,745 (+43675 +25.08%)
Cycles increased due to increased malloc calls.

Sorting Added (qsort) : 249,745 (+32000 +14.69%)

Added Write to disk : 293,772 (+44027 17.62%)

So seeing the jumps, the pipeline comes out to be

8th Sept 2016:

BASE_CASE : 99488 (Can't do anything)
READ_FILE : 74600
CREATE_OM : 43675
SORTING   : 32000
WRITE     : 44027
--------------------
Total     : 293790

ChangeList:
      1.Used a memory pool, instead of mallocing every time for the data. Set the pointers in it to save some malloc calls.
        Insights: Do one malloc, one file read and dont't dereference twice like a->b->c in a loop. Save once then reuse. esp.For loop
	Gain : 293790 - 254366 = 39424 (13.42%) 
