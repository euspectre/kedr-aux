include(kbuild_system)

set(kmodule_name "cp_controller")
set(kmodule "${kmodule_name}.ko")
# Should be configured in some way
set(arch "x86")

set(arch_dir "arch/${arch}")

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${arch_dir}/lib)

kbuild_include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${arch_dir}/include"
	"${CMAKE_CURRENT_BINARY_DIR}/${arch_dir}/lib"
	"${CMAKE_CURRENT_SOURCE_DIR}"
	)
kbuild_add_module(${kmodule_name} 
	"${CMAKE_CURRENT_BINARY_DIR}/controller.c"
    "${CMAKE_CURRENT_BINARY_DIR}/${arch_dir}/lib/inat.c"
    "${CMAKE_CURRENT_BINARY_DIR}/${arch_dir}/lib/insn.c"
    )

#add_custom_target(symvers ALL 
#            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Module.symvers)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Module.symvers
			DEPENDS ${kmodule_name})

#add_dependencies(${kmodule_name} symvers)

rule_copy_source("controller.c")
rule_copy_source("${arch_dir}/lib/inat.c")
rule_copy_source("${arch_dir}/lib/insn.c")

add_custom_target(inat_tables_file
			DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${arch_dir}/lib/inat-tables.c")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${arch_dir}/lib/inat-tables.c"
			COMMAND LC_ALL=C awk -f "${CMAKE_CURRENT_SOURCE_DIR}/${arch_dir}/tools/gen-insn-attr-x86.awk"
				"${CMAKE_CURRENT_SOURCE_DIR}/${arch_dir}/lib/x86-opcode-map.txt" >
				"${CMAKE_CURRENT_BINARY_DIR}/${arch_dir}/lib/inat-tables.c"
			DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${arch_dir}/lib/x86-opcode-map.txt"
			)

add_dependencies(${kmodule_name} inat_tables_file)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${kmodule}
		DESTINATION ${KEDR_INSTALL_PREFIX_KMODULE})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Module.symvers
		DESTINATION ${KEDR_INSTALL_PREFIX_KSYMVERS})