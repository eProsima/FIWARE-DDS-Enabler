import os
import re
import fnmatch

class IDLProcessor:
    def __init__(self):
        self.structs_info = set()

        # List of files to ignore
        self.files_to_ignore = {
            "declarations.idl",
            "external.idl",
            "member_id.idl", # Not support while @autoid(HASH) StructTypeFlag information cannot be pass to DynamicType API.
            "relative_path_include.idl"
        }
        # List of struct names to ignore
        self.struct_names_to_ignore = ["MapWString*", "MapInnerAliasBoundedWStringHelper*", "AnnotatedStruct", "foo"]
        if self.struct_names_to_ignore:
            print("Struct names to ignore:")
            print(self.struct_names_to_ignore)
    
        # List of IDL files that don't have a TypeObject
        self.idl_without_typeobjects = {"XtypesTestsTypeNoTypeObject", "declarations", "external"}
        if self.struct_names_to_ignore:
            print("IDL files file whitout TypeObject:")
            print(self.idl_without_typeobjects)

    def extract_structures(self, idl_text):
        # Regular expressions for module and struct extraction
        module_regexp = r'module\s+(\w+)\s*{((?:.|[\r\n])*?)\};\s*\};'
        struct_regexp = r'struct\s+(\w+)(\s*:\s*\w+)?\s*\{([^}]+)'

        # Extract structures within modules
        modules = re.findall(module_regexp, idl_text, re.DOTALL)        
        module_structures = {}
        for module_match in modules:
            module_name, module_content = module_match
            structures = re.findall(struct_regexp, module_content)
            module_structures[module_name] = structures

        # Find structures outside modules
        outside_structures = re.findall(struct_regexp, idl_text)
        
        # Remove duplicate structures
        for module_name, structures in module_structures.items():
            for structure in structures:
                if structure in outside_structures:
                    outside_structures.remove(structure)

        return module_structures, outside_structures

    def process_idl_files(self, idls_path):
        # Search for .idl files in the specified folder and its subdirectories
        print("Searching for .idl files...")
        for root, dirs, files in os.walk(idls_path):
            for file_name in files:
                if file_name.endswith('.idl') and file_name not in self.files_to_ignore:
                    file_path = os.path.join(root, file_name)
                    print(f"Found .idl file: {file_path}")
                    idl_file_relative_path = os.path.relpath(file_path, idls_path)
                    idl_file_name = os.path.splitext(idl_file_relative_path)[0]
                    with open(file_path, 'r') as file:
                        content = file.read()
                        module_structures, outside_structures = self.extract_structures(content)
                        
                        # Store struct information along with the IDL file name and module name
                        for module_name, structures in module_structures.items():
                            for structure in structures:
                                self.structs_info.add((structure[0], idl_file_name, module_name, idls_path))
                        
                        # Store struct information for structures outside modules
                        for structure in outside_structures:
                            self.structs_info.add((structure[0], idl_file_name, "", idls_path))

        
def update_types_header_file(structs_info):
    # Update types header file with necessary includes
    header_file_path = os.path.join(os.path.dirname(__file__), 'DdsEnablerTypedTestTypeHeaders.hpp')

    with open(header_file_path, 'r') as header_file:
        content = header_file.read()

    # Remove existing include lines
    content = re.sub(r'#include\s+".*?"\n', '', content)

    endif_index = content.rfind("#endif")

    # Add new include lines before the #endif directive
    new_include_lines = set()
    for _, idl_file_name, _, idls_path in structs_info:
        include_line = f'#include "types/{idl_file_name}PubSubTypes.hpp"\n'
        new_include_lines.add(include_line)

    content = content[:endif_index].rstrip() + '\n' + '\n' + ''.join(sorted(list(new_include_lines))) + '\n' + content[endif_index:]

    with open(header_file_path, 'w') as header_file:
        header_file.write(content)

    print(f"Header file '{header_file_path}' updated successfully.")


def update_tests_macros(structs_info, struct_names_to_ignore):
    # Update participant header file with necessary macros and type information
    script_dir = os.path.dirname(os.path.abspath(__file__))
    file_path = os.path.join(script_dir, "DdsEnablerTypedTest.cpp")
    temp_file_path = file_path + ".tmp"

    with open(file_path, 'r') as file:
        content = file.read()

    # Remove existing macros "DEFINE_DDSENABLER_TYPED_TEST"
    content = re.sub(r'\s*DEFINE_DDSENABLER_TYPED_TEST\([^)]*\);\s*', '', content)

    # Prepare new macro lines
    new_macro_lines = []
    structs_info = sorted(list(structs_info), key=lambda x: (x[1], x[0]))  # Sort structs_info alphabetically
    for struct_name, idl_file_name, module_name, _ in structs_info:
        if not any(fnmatch.fnmatch(struct_name, pattern) for pattern in struct_names_to_ignore):
            if module_name:
                new_macro_line = f"DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_{struct_name}, {module_name}::{struct_name}PubSubType);\n"
            else:
                new_macro_line = f"DEFINE_DDSENABLER_TYPED_TEST(ddsenabler_send_samples_{struct_name}, {struct_name}PubSubType);\n"
            new_macro_lines.append(new_macro_line)

    # Insert new macros after the specified line
    insertion_point = "// This macros are updated automatically using the update_headers_and_create_cases.py script"
    insertion_index = content.find(insertion_point) + len(insertion_point)

    # Combine content and new macros
    updated_content = content[:insertion_index] + '\n' + ''.join(new_macro_lines) + content[insertion_index:]

    with open(temp_file_path, 'w') as file:
        file.write(updated_content)

    os.replace(temp_file_path, file_path)

    print(f"Test macros in DdsEnablerTypedTest.cpp updated successfully.")

def update_tests_cmake(structs_info, struct_names_to_ignore):
    # Update the CMakeLists.txt with a new list of tests, excluding ignored struct names
    script_dir = os.path.dirname(os.path.abspath(__file__))
    cmake_file_path = os.path.join(script_dir, "CMakeLists.txt")
    temp_cmake_file_path = cmake_file_path + ".tmp"

    with open(cmake_file_path, 'r') as file:
        content = file.read()

    # Prepare new test list lines, filtering out ignored struct names
    new_test_lines = []
    structs_info = sorted(list(structs_info), key=lambda x: (x[1], x[0]))  # Sort structs_info alphabetically
    for struct_name, _, _, _ in structs_info:
        if not any(fnmatch.fnmatch(struct_name, pattern) for pattern in struct_names_to_ignore):
            new_test_line = f"    ddsenabler_send_samples_{struct_name}\n"
            new_test_lines.append(new_test_line)

    # Replace the existing set(TEST_LIST ...) block with the new test list
    test_list_pattern = r'set\(TEST_LIST\s*\n(.*?)\)'
    new_test_list_block = f"set(TEST_LIST\n{''.join(new_test_lines)})"
    updated_content = re.sub(test_list_pattern, new_test_list_block, content, flags=re.DOTALL)

    # Write the updated content to a temporary file
    with open(temp_cmake_file_path, 'w') as file:
        file.write(updated_content)

    # Replace the original CMakeLists.txt file with the updated one
    os.replace(temp_cmake_file_path, cmake_file_path)

    print(f"Test list in CMakeLists.txt updated successfully.")


def main():
    print("This script updates the types header file and creates macros to test the all the structures found in the IDL files.")

    processor = IDLProcessor()
    processor.process_idl_files("../resources/dds-types-test/IDL")

    if not processor.structs_info:
        print("No structures found in the IDL files.")
        print("Expected to find them in: ../resources/dds-types-test/IDL")
        return

    update_types_header_file(processor.structs_info)
    update_tests_macros(processor.structs_info, processor.struct_names_to_ignore)
    update_tests_cmake(processor.structs_info, processor.struct_names_to_ignore)


if __name__ == "__main__":
    main()
