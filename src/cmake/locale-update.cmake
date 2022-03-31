find_package(Gettext)

include(../data/locale/sources.cmake)

get_filename_component(DUNE_LOCALE_DIR "../data/locale/" ABSOLUTE)

function(make_generated_dir path output_var native_output_var)
    get_filename_component(_dir "${path}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

    file(MAKE_DIRECTORY "${_dir}")
    file(TO_NATIVE_PATH "${_dir}" _native_dir)

    set(${output_var} "${_dir}" PARENT_SCOPE)
    set(${native_output_var} "${_native_dir}" PARENT_SCOPE)
endfunction()

function(generate_po po_name pot_file output_dir output_po_var)
    get_filename_component(_po_file "${po_name}" ABSOLUTE BASE_DIR ${DUNE_LOCALE_DIR} )
    get_filename_component(_output_po "${po_name}" ABSOLUTE BASE_DIR "${output_dir}")
    file(TO_NATIVE_PATH "${_output_po}" _output_po_native)
    file(TO_NATIVE_PATH "${pot_file}" pot_file_native)
    file(TO_NATIVE_PATH "${_po_file}" _po_file_native)

    add_custom_command(
        OUTPUT "${_output_po}"
        COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet -o "${_output_po_native}" "${_po_file_native}" "${pot_file_native}"
        DEPENDS "${pot_file}" "${_po_file}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    set(${output_po_var} "${_output_po}" PARENT_SCOPE)
endfunction()

if(GETTEXT_FOUND)
    if(GETTEXT_XGETTEXT_EXECUTABLE)
    else()
        get_filename_component(GETTEXT_DIR GETTEXT_MSGFMT_EXECUTABLE DIRECTORY)
        find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext
            HINT ${GETTEXT_DIR}
        )
    endif()

    if(GETTEXT_MSGATTRIB_EXECUTABLE)
    else()
        get_filename_component(GETTEXT_DIR GETTEXT_MSGFMT_EXECUTABLE DIRECTORY)
        find_program(GETTEXT_MSGATTRIB_EXECUTABLE msgattrib
            HINT ${GETTEXT_DIR}
        )
    endif()

    if(GETTEXT_XGETTEXT_EXECUTABLE AND GETTEXT_MSGATTRIB_EXECUTABLE)
        ##########
        # Generate dunelegacy.pot
        ##########

        make_generated_dir("locale" binary_locale_dir native_binary_locale_dir)
        make_generated_dir("tmp" binary_tmp_dir native_binary_tmp_dir)

        get_filename_component(generated_pot "dunelegacy.pot" ABSOLUTE BASE_DIR "${binary_locale_dir}")
        file(TO_NATIVE_PATH "${generated_pot}" generated_pot_native)

        get_filename_component(src_pot "dunelegacy.pot" ABSOLUTE BASE_DIR "${DUNE_LOCALE_DIR}")

        set(XGETTEXT_SOURCES "")
        set(XGETTEXT_ABSOLUTE_SOURCES "")

        foreach(f ${SOURCES} ${EXE_SOURCES} ${HEADERS} ${EXE_HEADERS})
            get_filename_component(f_absolute "${f}" ABSOLUTE)
            list(APPEND XGETTEXT_ABSOLUTE_SOURCES "${f_absolute}")
            file(RELATIVE_PATH f_src_relative "${CMAKE_SOURCE_DIR}" "${f_absolute}")
            file(TO_NATIVE_PATH "${f_src_relative}" f_src_relative_native)
            list(APPEND XGETTEXT_SOURCES "${f_src_relative_native}")
        endforeach()

        list(SORT XGETTEXT_SOURCES)

        set(XGETTEXT_OPTIONS
			--language=C++
			--keyword=_
			--add-comments=/
			--package-name=dunelegacy
			"--package-version=${CMAKE_PROJECT_VERSION}"
		)

        add_custom_command(OUTPUT "${generated_pot}"
            COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE} ${XGETTEXT_OPTIONS} -o "${generated_pot_native}" ${XGETTEXT_SOURCES}
            DEPENDS ${XGETTEXT_ABSOLUTE_SOURCES}
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        )

        ##########
        # Generate new .po files from the generated dunelegacy.pot
        ##########

        # This works, but unfortunately GETTEXT_CREATE_TRANSLATIONS() sets the "sort" flag.
        #GETTEXT_CREATE_TRANSLATIONS(dunelegacy.pot ${PO_SOURCES})

        set(generated_po_files "")

        foreach(src_po ${PO_SOURCES})
            generate_po(${src_po} "${generated_pot}" "${binary_locale_dir}" generated_po)
            list(APPEND generated_po_files "${generated_po}")
        endforeach()

        generate_po(${ENGLISH_PO} "${generated_pot}" "${binary_tmp_dir}" intermediate_english_po)

        set(generated_english_po "${binary_locale_dir}/${ENGLISH_PO}")
        file(TO_NATIVE_PATH "${generated_english_po}" generated_english_po_native)

        file(TO_NATIVE_PATH "${intermediate_english_po}" intermediate_english_po_native)

        add_custom_command(
            OUTPUT "${generated_english_po}"
            COMMAND ${GETTEXT_MSGATTRIB_EXECUTABLE} --translated --no-fuzzy -o "${generated_english_po_native}" "${intermediate_english_po_native}"
            DEPENDS "${intermediate_english_po}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )

        list(APPEND generated_po_files "${generated_english_po}")

        add_custom_target(generate_locale ALL DEPENDS ${generated_po_files})

        ##########
        # Add the update_locale target to actually modify the source locale files.
        ##########

        get_filename_component(_update_locale_script "update_locale.cmake" ABSOLUTE BASE_DIR "${binary_tmp_dir}")

        file(WRITE "${_update_locale_script}"
            "configure_file(\"${generated_pot}\" \"${src_pot}\" COPYONLY)\n"
        )

        foreach(generated_po ${generated_po_files})
            get_filename_component(po_name "${generated_po}" NAME)
            get_filename_component(src_po "${po_name}" ABSOLUTE BASE_DIR "${DUNE_LOCALE_DIR}")
            file(APPEND "${_update_locale_script}"
                "configure_file(\"${generated_po}\" \"${src_po}\" COPYONLY)\n"
            )
        endforeach()

        get_filename_component(_update_stamp "update_locale.stamp" ABSOLUTE BASE_DIR "${binary_tmp_dir}")

        add_custom_command(OUTPUT "${_update_stamp}"
            COMMAND ${CMAKE_COMMAND} -P "${_update_locale_script}"
            COMMAND ${CMAKE_COMMAND} -E touch "${_update_stamp}"
            DEPENDS generate_locale
            COMMENT "Updating locale"
        )

        message(STATUS "Adding update_locale target")

        add_custom_target(update_locale DEPENDS "${_update_stamp}")
    else()
        message(WARNING "Unable to add locale targets")
    endif()
else()
    message(STATUS "Gettext not found")
endif()

