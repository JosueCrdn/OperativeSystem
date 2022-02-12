#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <mmu.h>

#define NUMPROCS 4
#define PAGESIZE 4096
#define PHISICALMEMORY 12*PAGESIZE 
#define TOTFRAMES PHISICALMEMORY/PAGESIZE
#define RESIDENTSETSIZE 3
extern char *base;
extern int framesbegin;
extern int idproc;
extern int systemframetablesize;
extern int ptlr;

extern struct SYSTEMFRAMETABLE *systemframetable;
extern struct PROCESSPAGETABLE *ptbr;


int get_lru();
int getfreeframe();
int getvirtualframe();

int pagefault(char *vaddress)
{
    int i;
    int frame,vframe; 
    long pag_a_expulsar;
    int fd;
    char buffer[PAGESIZE];
    int pag_del_proceso;

    // A partir de la dirección que provocó el fallo, calculamos la página
    pag_del_proceso=(long) vaddress>>12;
    vframe=ptbr[pag_del_proceso].framenumber;

    // Si la página del proceso está en un marco virtual del disco    
    if(ptbr[pag_del_proceso].framenumber >= framesbegin+TOTFRAMES)
    {
        // Lee el marco virtual al buffer
        readblock(buffer, vframe);
        
        // Libera el frame virtual
        systemframetable[vframe].assigned=0;
    }
    // Cuenta los marcos asignados al proceso
    i=countframesassigned();

    // Si ya ocupó todos sus marcos, expulsa una página
    if(i>=RESIDENTSETSIZE)
    {
        // Buscar una página a expulsar
        pag_a_expulsar = get_lru();

        // La pagina ya no estara presente 
        ptbr[pag_a_expulsar].presente=0;
        frame=ptbr[pag_a_expulsar].framenumber;

        // Si la pagina ya fue modificada, grabala en disco
        if(ptbr[pag_a_expulsar].modificado != 0){
            saveframe(frame);
            ptbr[pag_a_expulsar].modificado=0;
        }

        // Buscar un frame en memoria secundaria
        vframe=getvirtualframe();
        if(vframe==-1){
            return(-1);
        }

        //Copia el frame a memoria secundaria y actualiza la tabla
        copyframe(frame,vframe);
        ptbr[pag_a_expulsar].framenumber=vframe;
        ptbr[pag_a_expulsar].presente=0;
        systemframetable[frame].assigned=0;

    } 

    // Busca un marco libre en el sistema
    frame=getfreeframe();
    if(frame==-1){
        return (-1); // Regresar indicando error de memoria insuficiente
    }

    // Si la pagina estaba en memoria secundaria
    if(ptbr[pag_del_proceso].framenumber>=framesbegin+TOTFRAMES){
        writeblock(buffer, frame);
        loadframe(frame);
    }

    // Poner el bit de presente en 1 en la tabla de paginas y frame
    ptbr[pag_del_proceso].modificado=0;
    ptbr[pag_del_proceso].presente=1;
    ptbr[pag_del_proceso].framenumber=frame;

    return(1); // Todo chido
}


int get_lru()
{
    // Recorrer la tabla de páginas del proceso buscando la página que hace más
    // tiempo no es referida
    unsigned long t = 0xFFFFFFFFFFFFFFFF;
    int i;
    int p=0;
    for(i=0;i<6;i++){
        if((ptbr+i)->presente==1){
	    if((ptbr+i)->tlastaccess < t)
        {
            t = (ptbr+i)->tlastaccess;
            p = i;
        }}}
    return(p);
}

// Busca un marco FÍSICO (del 0 al 12) disponible en la tabla de marcos del sistema
// systemframetable
int getfreeframe()
{
    int i;
    // Busca un marco libre en el sistema
    for(i=framesbegin;i<systemframetablesize+framesbegin;i++)
        if(!systemframetable[i].assigned)
        {
            systemframetable[i].assigned=1;
            break;
        }
    if(i<systemframetablesize+framesbegin){
        systemframetable[i].assigned=1;
    }
    else{
        i=-1;
    }
    return(i);
}

// Busca un marco VIRTUAL (del 12 al 23) disponible en la tabla de marcos del sistema
// systemframetable
int getvirtualframe()
{
    int i;
    // Busca un marco virtual libre en el sistema
    for(i = systemframetablesize+framesbegin; i<2*(systemframetablesize)+framesbegin; i++)
        if(!systemframetable[i].assigned)
        {
            systemframetable[i].assigned=1;
            break;
        }
    if(i<2*(systemframetablesize)+framesbegin){
        systemframetable[i].assigned=1;
    }
    else{
        i=-1;
    }
    return(i);
}

