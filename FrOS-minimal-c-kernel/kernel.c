void main() {
    char* video_memory = (char*) 0xb8000;
    *video_memory = 'L';
}

/*
segmentation - required for protected mode
paging - optional, but highly recommended
 */