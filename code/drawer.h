#ifndef DRAWER_H 
#define DRAWER_H
typedef struct candy
{
	void* data;
	struct candy *next;
}candy, *qcandy;

typedef struct
{
	candy* head;
}drawer,*qdrawer;

void InitDrawer(qdrawer qdraw); 
int DrawerEmpty(qdrawer qdraw);  
int DrawerLength(qdrawer qdraw);  
int GetHead(qdrawer qdraw, qcandy qcan);  
int EnDrawer(qdrawer qdraw, qcandy qcan);  
int DeDrawer(qdrawer qdraw, qcandy qcan);  
#endif 
