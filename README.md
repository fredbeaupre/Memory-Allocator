Does not pass all tests. Used the `list_details` function to show outputs and debug.

In worst fit, it seems that before the calls to sma_free(),  the algorithm does well in finding
the largest block in the free list and allocating it accordingly, as well as creating a new free block from the
excess size. The problem seems to appear after the calls to sma_free(). After that, the algorithm fails to find a
suitable block in the free list and allocates the memory for the new sma_malloc() request by increasing the program break, which causes test 3 to output a failure.

To compile: 
    $ make sma

To run:
    $./sma
