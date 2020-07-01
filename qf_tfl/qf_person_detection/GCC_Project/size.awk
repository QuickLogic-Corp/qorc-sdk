/^\.text /	{text = $2 }
/^ \.fini /	{fini = $2 }
/^\.data/	{data = last 
		 ram = $2}
/^\.heap/	{heap = $2 }
/__StackTop/	{stop = $1 }
/__StackLimit =/	{slim = $1 }
/ucHeap/	{ucHeap = $3}

		{ if(NF > 1) last = $2 }

END		{ print "textStart=", text
		  print "textEnd=  ", fini
       		  print "dataEnd=  ", data
		  print "ramStart= ", ram
       		  print "heapEnd=  ", heap
       		  print "stackBot= ", slim
       		  print "stackTop= ", stop
		  print "ucHeap=   ", ucHeap
  		}
