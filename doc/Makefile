# Uses pandoc to generate nice PDFs from our markdown. See https://pandoc.org/

awsm.pdf: README.md design.md abi.md
	pandoc $^ -o awsm.pdf

# The default PDF renderer chokes on Unicode characters, so replacing ✓ with X
# ✓ is equivalent to 0xE2 0x9C 0x93
README.md: ../README.md
	cat ../README.md | sed 's/\xe2\x9c\x93/X/g' > README.md

clean: 
	rm *.pdf README.md
