## Segmentation
* each proccess is atomic (can't be split up)
* RAM is divided into blocks of **fixed** size (can't be split up)


## Paging
* RAM is divided into blocks of **same** size
* logical memory => page table => physical memory (physical memory adress)
* each program can behvae like it has the whole memmory to itself
* optional for protected mode

See this example:  
* 1st image is paging
* 2nd is segmentation


![IMG_7749](https://github.com/filipvabrousek/osdev/assets/18376136/72b0758b-2f72-4a2a-8467-79e4a2d26255)

![IMG_7750](https://github.com/filipvabrousek/osdev/assets/18376136/1879460d-9250-4686-b3ef-78b7c6b38dcd)


## 32-bit Protected mode

* allows adressing memory up to 4GB
* we can implement pagign and multitasking (we are not doing this)
* we can use C
* we cannot use BIOS functions anymore
* we are using flat memory model not really a segmentation
* setting up segments => mandatory
* => we createGDT (Global Descriptor Table)
