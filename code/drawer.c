#include<stdio.h>
#include<stdlib.h>
#include"drawer.h"
void InitDrawer(drawer* mp)
{
	mp->head = (qcandy)malloc(sizeof(candy));
	mp->head->next = NULL;
}

int DrawerEmpty(drawer* mpp)
{
	if(DrawerLength(mpp) == 0)
		return 1;
	return 0;
}

int DrawerLength(drawer* mpp)
{
	candy doc;
	qcandy p = &doc;
	p->next = mpp->head->next;
	int i = 0;
	while(p->next != NULL)
	{
		p = p->next;
		i++;
	}
	return i;
}

int GetHead(drawer *mpp, candy *doc)
{
	if(mpp->head->next == NULL)
		return 0;
	doc->data = mpp->head->data;  
	doc->next = mpp->head->next;
}

int EnDrawer(drawer *mpp, qcandy doc)
{
	doc->next = mpp->head->next;
	mpp->head->next = doc;
	return 1;
}

int DeDrawer(drawer *mpp, candy *doc)
{
	candy docu;
	qcandy p = &docu;
	p->next = mpp->head;
	while(p->next->next != NULL)
	{
		if(p->next->next == doc)
		{
			p->next->next = doc->next;
			free(doc);
			return 1;
		}
		p->next = p->next->next;
	}
	return 0;
}

