#pragma once



struct Picture
{
    int picId;
    int picSize;
    int *pic;
    int matchCount;
    int matchPlace[3][3];
};

struct Objects
{
    int objId;
    int objSize;
    int *obj;
};
