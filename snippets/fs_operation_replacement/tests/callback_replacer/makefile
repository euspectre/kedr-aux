all:
	make -C usp_replacer/ all
	make -C process_user_string/ all
	make -C user/ all
	make -C payload/ all
clean:
	make -C payload/ clean
	make -C user/ clean
	make -C process_user_string/ clean
	make -C usp_replacer/ clean
check:
	@if sh test.sh "   "; then echo "Test passed."; else echo "Test failed."; fi

.PHONY: all clean check
