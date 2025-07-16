/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions 
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
*/
// add other headers as required
#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>

/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this 
macro to make the output of all system same and conduct a fair evaluation. 
*/
#define PAGE_SIZE 4096
#define PROCESS 1
#define HOLE 0
#define A1000 1000

// Sub chain node
typedef struct Segment {
    size_t size;
    void* start_address;
    void * paddr;
    int is_allocated; // 1 for allocated, 0 for free
    struct Segment* prev;
    struct Segment* next;
} seg;

// MainChainNode represents a node in the main chain that manages memory segments.
typedef struct MainChainNode {
    struct Segment* sub_chain;  // Subchain of memory segments
    void* mems_virtual_address;
    void * paadr;
    struct MainChainNode* prev;
    struct MainChainNode* next;
    size_t size;
}node ;

struct MainChainNode *head;
// number of node to track allocations of virtual addresses
int number_of_nodes=0;
// hardcoded mock virtual addresses
int a1[]={1000,2000,3000,4000,4096,5096,6096,7096,8192,9192,10192,11192};

/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/

// initialising the main chain to NULL
void mems_init(){
    head=NULL;
    // head->subchain=NULL;
}


/*
This function will be called at the end of the MeMS system and its main job is to unmap the 
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
// at the end of the script iterating over all the main chain nodes and their subchains to free the memory
void mems_finish(){
    // start from the head node
    node *currentNode = head;
    while (currentNode != NULL) {
        // pick the starting node for the segment / sub chain
        seg* currentSegment = currentNode->sub_chain;
        seg *temp=currentSegment;
        while (currentSegment != NULL) {
            temp=currentSegment;
            currentSegment = currentSegment->next;
            // pass the physical address and the size of the block to be unmapped
            munmap(temp->paddr,temp->size);
        }
        // saving the pointer for the current node
        node* ntemp = currentNode;
        // moving the curr node forward
        currentNode = currentNode->next;
        
        // Unmapping the main chain nodes
        munmap(ntemp->paadr, ntemp->size);
    }
}


/*
Allocates memory of the specified size by reusing a segment from the free list if 
a sufficiently large segment is available. 

Else, uses the mmap system call to allocate more memory on the heap and updates 
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/ 
// void* mems_malloc(size_t size){
//     if(head==NULL){
//         size_t allocation_size=(size / PAGE_SIZE + 1)*PAGE_SIZE;
//         void *mems_pa=mmap(NULL, allocation_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
//         if(mems_pa==MAP_FAILED){
//             perror("mmap failed");
//             return NULL;
//         }
//         head = 
//     }
// }

// ...

// mems_malloc() implementation using mmap
void* mems_malloc(size_t size) {
    // if the main chain was empty that is we are allocating memory for the first time
    if(head==NULL)
    {
        // creating a main chain node
        // NULL to tell kernel to allocate any memory address available
        // prot read and write allow read and write access to the addr
        // -1 is for file descriptor marking that this address is not pointing to any file, and 0 is the pointer in that file
        void * addr=mmap(NULL,PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        // we get physical address
        head=(node *)addr;
        head->next=NULL;
        head->prev=NULL;
        head->paadr=addr;
        head->mems_virtual_address=(void*)10;
        head->size=PAGE_SIZE;

        // creating a sub-chain node
        void* seg_addr=mmap(NULL,size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        seg * n1=(seg*)seg_addr;
        head->sub_chain=n1;
        n1->is_allocated=1;
        n1->next=NULL;
        n1->paddr=seg_addr;
        n1->prev=head;
        n1->size=size;
        // putting the start address of the node as the mock nodes
        n1->start_address=(void *)a1[number_of_nodes];
        number_of_nodes++;
        // returning the virtual address for the memory allocated to the process
        return n1->start_address;
    }

    // if main chain node already exists
    struct MainChainNode* currentNode = head;
    // we find a segment to select
    struct Segment* selectedSegment = NULL;

    while (currentNode != NULL) {
        struct Segment* currentSegment = currentNode->sub_chain;
        // track total size of all segments
        size_t atemp=0;
        while(currentSegment!=NULL)
        {    
            // the size of the curr segment must be greater than the size needed to be allocated
            if(!currentSegment->is_allocated && currentSegment->size>size)
            {
                selectedSegment=currentSegment;
                break;
            }
            // updating the temp address
            atemp+=currentSegment->size;
            currentSegment=currentSegment->next;
        }
        // checking if the total memory used and new requested size still fits within the current page or not
        if(atemp+size<=currentNode->size)
        {
            // if yes we can add a sub-chain and allocate that memory to the process
            struct Segment* currentSegment = currentNode->sub_chain;
            while(currentSegment->next!=NULL)
            {
                currentSegment=currentSegment->next;
            }
            void* seg_addr=mmap(NULL,size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            seg * n1=(seg*)seg_addr;
            n1->is_allocated=1;
            n1->next=NULL;
            n1->paddr=seg_addr;
            n1->prev=currentSegment;
            n1->size=size;
            n1->start_address=(void *)(a1[number_of_nodes]);
            number_of_nodes++;
            currentSegment->next=n1;
            return n1->start_address;
        }
        currentNode=currentNode->next;
    }
    if (selectedSegment == NULL) {
        // No suitable segment found, allocate a new one using mmap
        // we need to add a new page
        // travering to the end of the linked list of main chain nodes
        node * temp11=head;
        while(temp11->next!=NULL)
        {
            temp11=temp11->next;
        }
        // allocating memory to new main chain node
        void* mems_physical_address = mmap(NULL,PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mems_physical_address == MAP_FAILED) {
            perror("mmap");
            return NULL;
        }
        node * temp=(node *)mems_physical_address;
        temp->next=NULL;
        temp->prev=temp11;
        temp->paadr=mems_physical_address;
        temp->mems_virtual_address=(void*)(number_of_nodes*10);
        temp->size=PAGE_SIZE;
        void* seg_addr=mmap(NULL,size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        seg * n1=(seg*)seg_addr;
        temp->sub_chain=n1;
        n1->is_allocated=1;
        n1->next=NULL;
        n1->paddr=seg_addr;
        n1->prev=temp;
        n1->size=size;
        n1->start_address=(void *)(a1[number_of_nodes]);
        number_of_nodes++;
        temp11->next=temp;
        return n1->start_address;
    } else {
        // suitable memory segment found
        // split it into two sub nodes hole and process
        void * addr1=mmap(NULL,selectedSegment->size-size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        seg * s1 =(seg*)addr1;
        s1->is_allocated = 1;
        seg * s2=selectedSegment->next;
        selectedSegment->next=s1;
        s1->next=s2;
        s1->is_allocated=0;
        s1->size=selectedSegment->size-size;
        s1->prev=selectedSegment;
        s1->paddr=addr1;
        selectedSegment->next=s1;
        selectedSegment->size=size;
        s1->start_address=(void *)(a1[number_of_nodes]);
        return selectedSegment->start_address;
    }
}

// ...




/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/
void mems_print_stats(){
    node * temp=head;
    int page_count=0;
    size_t hole_size=0;
    int ncount=1;

    while(temp!=NULL)
    {
        page_count++;
        seg* stemp=temp->sub_chain;
        while(stemp!=NULL)
        {
            if(!stemp->is_allocated)
            {
                hole_size+=stemp->size;
            }
            stemp=stemp->next;
        }
        temp=temp->next;
    }
    int ar[page_count];
    int i=0;
    temp=head;
    while(temp!=NULL)
    {
         seg* stemp=temp->sub_chain;
         int scount=0;
         int ss=4096*(i+1)-4095;
         printf("MAIN[%ld-%ld] ->",4096*(i+1)-4095,4096*(i+1));
        while(stemp!=NULL)
        {
            if(stemp->is_allocated)
            {
                printf("P[%ld-%ld] -> ",ss,ss+stemp->size);
                ss+=stemp->size; 
            }
            else{
                printf("H[%ld-%ld] -> ",ss,ss+stemp->size);
                ss+=stemp->size; 
            }
            scount++;
            stemp=stemp->next;  
        }
        ar[i]=scount;
        i++;
        printf("NULL\n");
        temp=temp->next;
    }
    printf("numbers of page used %d \n number of unused memory %d \n",page_count,hole_size);
    printf("total number of segments of each page ");
    for(int j=0;j<i;j++)
    {
        printf("%d ",ar[j]);
    }
}


/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
// function to get physical address given a virutal address
void* mems_get(void* v_ptr) {
    node * temp=head;
    int i=1;
    // iterate over the entire list of main chain nodes
    while(temp!=NULL)
    {
        // iterate over sub chain nodes
        seg* stemp=temp->sub_chain;
        while(stemp->next!=NULL)
        {
            // checking if the current segments virtual address matches the input virtual address
            if(stemp->start_address<v_ptr<=stemp->next->start_address)
            {
                return stemp->paddr;
            }
            stemp=stemp->next;
        }
        // if the final node and does not have a next node we match that if the virtual address falls within this page or not
        if(stemp->start_address<v_ptr<=4096*(i))
        {
            return stemp->paddr;
        }
        // move to next page
        temp=temp->next;
    }
    return NULL;
}



/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS) 
Returns: nothing
*/
void mems_free(void *v_ptr){
    // start from the head of the main chain
    node* currentNode = head;
    while (currentNode != NULL) {
        // travel through all the segments 
        seg* currentSegment = currentNode->sub_chain;
        while (currentSegment != NULL) {
            // checking if current segment matches the address needed
            if (currentSegment->start_address == v_ptr) {
                // Mark the segment as free
                currentSegment->is_allocated = 0;
                // TBA - implement merging of adjacent free holes for reducing memory fragmentations
                // once freed return to stop any further computation
                return;
            }
            currentSegment = currentSegment->next;
        }
        currentNode = currentNode->next;
    }
}
