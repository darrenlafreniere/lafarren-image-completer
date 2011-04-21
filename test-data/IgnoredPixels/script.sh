#This script was used to produce the output images in this directory from the inputs and masks.

#Full mask:
#This example uses a mask with only black (unknown) and white (known) pixels. The result is that the hole in the red part of the image gets filled with red, as we would expect.
ImageCompleter -ii input.png -im mask-full.png -io output-full.png

#Ignored pixel mask:
#This example demonstrates how to use "Ignored" pixels. The mask contains black (unknown), white (known) and gray (ignored) pixels. We have contrived this example such that the hole in the red region does not have any red pixels to use in the completion (the "known" red pixels are marked as ignored). Therefore, the hole is filled with the only pixels available, blue pixels.
ImageCompleter -ii input.png -im mask-ignored.png -io output-ignored.png