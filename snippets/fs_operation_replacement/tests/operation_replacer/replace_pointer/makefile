all:
	make -C delegate_operation_replacer/ all
	make -C base/ all
	make -C base_update_replacement/ all
	make -C user/ all
	make -C user_update_operations/ all
	make -C payload/ all
	make -C payload_special/ all
	make -C payload_special1/ all
clean:
	make -C payload_special1/ clean
	make -C payload_special/ clean
	make -C payload/ clean
	make -C user_update_operations/ clean
	make -C user/ clean
	make -C base_update_replacement/ clean
	make -C base/ clean
	make -C delegate_operation_replacer/ clean
check:
	@if sh test.sh "   "; then echo "Test passed."; else echo "Test failed."; fi

.PHONY: all clean check
