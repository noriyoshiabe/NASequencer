#include <stdio.h>
#include "NASet.h"
#include "NAMap.h"

int main(int argc, char **argv)
{
    NASet *set = NASetCreate(NAHashCString, NADescriptionCString);
    NASetAdd(set, "TEST1");
    NASetAdd(set, "TEST2");
    NASetAdd(set, "TEST3");
    printf("add=%d\n", NASetAdd(set, "TEST4"));
    printf("add=%d\n", NASetAdd(set, "TEST4"));
    printf("count=%d\n", NASetCount(set));
    NASetRemove(set, "TEST3");
    NASetDump(set);

    printf("contains TEST3=%d\n", NASetContains(set, "TEST3"));
    printf("contains TEST2=%d\n", NASetContains(set, "TEST2"));

    printf("count=%d\n", NASetCount(set));
    printf("get=%s\n", NASetGet(set, "TEST1"));

    NASetAdd(set, "hoge");
    NASetAdd(set, "foo");
    NASetAdd(set, "namidi");
    NASetAdd(set, "testtest");
    NASetAdd(set, "wiwkd;f");
    NASetAdd(set, "ldagl;lkfa");
    NASetAdd(set, "dsaf/lkbfa");
    NASetAdd(set, "dasf;laksdf");
    NASetAdd(set, "dddddld");
    NASetAdd(set, "ldsa;ro49kf");
    NASetAdd(set, "sa.ldfsa");
    NASetAdd(set, "asafa");
    NASetAdd(set, "scratch");

    NASetDump(set);

    NASetDestroy(set);

    NAMap *map = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionCString);
    NAMapPut(map, "TEST1", "TEST1-Value");
    NAMapPut(map, "TEST2", "TEST2-Value");
    NAMapPut(map, "TEST3", "TEST3-Value");
    printf("count=%d\n", NAMapCount(map));
    NAMapRemove(map, "TEST3");
    NAMapDump(map);

    printf("contains key TEST3=%d\n", NAMapContainsKey(map, "TEST3"));
    printf("contains key TEST2=%d\n", NAMapContainsKey(map, "TEST2"));

    printf("count=%d\n", NAMapCount(map));
    printf("get=%s\n", NAMapGet(map, "TEST1"));

    NAMapPut(map, "example1", "hoge");
    NAMapPut(map, "example2", "foo");
    NAMapPut(map, "example3", "namidi");
    NAMapPut(map, "example4", "testtest");
    NAMapPut(map, "example5", "wiwkd;f");
    NAMapPut(map, "example6", "ldagl;lkfa");
    NAMapPut(map, "example7", "dsaf/lkbfa");
    NAMapPut(map, "example8", "dasf;laksdf");
    NAMapPut(map, "example9", "dddddld");
    NAMapPut(map, "example10", "ldsa;ro49kf");
    NAMapPut(map, "example11", "sa.ldfsa");
    NAMapPut(map, "example12", "asafa");
    NAMapPut(map, "example13", "scratch");

    NAMapDump(map);

    NAMapDestroy(map);
    return 0;
}
