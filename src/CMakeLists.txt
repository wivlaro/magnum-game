find_package(Corrade REQUIRED Main)

if (CORRADE_TARGET_EMSCRIPTEN)
    find_package(Magnum REQUIRED GL EmscriptenApplication
            MeshTools Shaders SceneGraph Trade Primitives TextureTools Text DebugTools)
else ()
    find_package(Magnum REQUIRED GL Sdl2Application
            MeshTools Shaders SceneGraph Trade Primitives TextureTools Text DebugTools)
endif ()
find_package(MagnumIntegration REQUIRED Bullet)
find_package(MagnumPlugins REQUIRED StbImageImporter GltfImporter StbTrueTypeFont)
find_package(Bullet REQUIRED Dynamics)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

corrade_add_resource(MagnumGameApp_RESOURCES resources.conf)

add_executable(MagnumGameApp MagnumGameApp.cpp
        RigidBody.cpp
        RigidBody.h
        MagnumGameCommon.h
        Player.cpp
        Player.h
        TexturedDrawable.cpp
        TexturedDrawable.h
        ${MagnumGameApp_RESOURCES}
        IEnableDrawable.h
        MagnumGameApp.h
        DebugLines.cpp
        DebugLines.h
        MagnumGameApp_ui.cpp
        MagnumGameApp_input.cpp
        Tweakables.cpp
        Tweakables.h
        GameState.cpp
        GameState.h
        GameAssets.cpp
        GameAssets.h
        Animator.cpp
        Animator.h
        CameraController.cpp
        CameraController.h
        AnimatorAsset.cpp
        AnimatorAsset.h
        UserInterface.cpp
        UserInterface.h
        GameShader.cpp
        GameShader.h
        ShadowCasterDrawable.cpp
        ShadowCasterDrawable.h
        ShadowLight.cpp
        ShadowLight.h
        ShadowCasterShader.h
        ShadowCasterShader.cpp
)
target_link_libraries(MagnumGameApp PRIVATE
        Corrade::Main
        Magnum::Application
        Magnum::GL
        Magnum::Magnum
        Magnum::MeshTools
        Magnum::Primitives
        Magnum::SceneGraph
        Magnum::Shaders
        Magnum::Trade
        Magnum::TextureTools
        Magnum::Text
        Magnum::DebugTools
        MagnumIntegration::Bullet
        Bullet::Dynamics)


# Make the executable a default target to build & run in Visual Studio
set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MagnumGameApp)

set_target_properties(MagnumGameApp PROPERTIES CXX_STANDARD 17)


if (CORRADE_TARGET_EMSCRIPTEN)

    include(UseEmscripten)
    emscripten_embed_file(MagnumGameApp ../font/Roboto-Regular.ttf /font/Roboto-Regular.ttf)
    emscripten_embed_file(MagnumGameApp ../models/characters/ /models/characters/)
    emscripten_embed_file(MagnumGameApp ../models/levels/ /models/levels/)
    emscripten_embed_file(MagnumGameApp ../shaders/ /shaders/)

#    add_custom_command(
#            OUTPUT generated_dummy_embeds
#            COMMAND ${CMAKE_COMMAND} -E touch generated_dummy_embeds
#            DEPENDS ../shaders/*.vert ../shaders/*.frag ../models/characters/* ../models/levels/* ../font/Roboto-Regular.ttf
#    )
#
#    add_custom_target(dummy_embed_target DEPENDS generated_dummy_embeds)
#    add_file_dependencies()
    file(GLOB EMBEDDED_FILES ../shaders/*.vert ../shaders/*.frag ../models/characters/* ../models/levels/* ../font/Roboto-Regular.ttf)
    set_property(SOURCE MagnumGameApp APPEND PROPERTY OBJECT_DEPENDS ${EMBEDDED_FILES})



    target_link_libraries(MagnumGameApp PRIVATE
            MagnumPlugins::StbImageImporter
            MagnumPlugins::GltfImporter
            MagnumPlugins::StbTrueTypeFont)


    # EXTRA_EXPORTED_RUNTIME_METHODS got renamed to EXPORTED_RUNTIME_METHODS in
    # Emscripten 2.0.18
    if (EMSCRIPTEN_VERSION VERSION_LESS 2.0.18)
        target_link_options(MagnumGameApp PRIVATE "SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS=[\"ccall\"]")
    else ()
        target_link_options(MagnumGameApp PRIVATE "SHELL:-s EXPORTED_RUNTIME_METHODS=[\"ccall\"]")
    endif ()
    target_link_options(MagnumGameApp PRIVATE
            "SHELL:$<$<CONFIG:Release>:--closure 1>"
            "SHELL:-s ALLOW_MEMORY_GROWTH=1"
            "SHELL:-s EXPORTED_FUNCTIONS=[\"_main\",\"_malloc\"]")


    add_custom_command(TARGET MagnumGameApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MAGNUM_EMSCRIPTENAPPLICATION_JS}
            ${MAGNUM_WEBAPPLICATION_CSS}
            ${CMAKE_CURRENT_SOURCE_DIR}/index.html
            $<TARGET_FILE_DIR:MagnumGameApp>)

    install(TARGETS MagnumGameApp DESTINATION ${MAGNUM_DEPLOY_PREFIX})
    install(FILES
            index.html
            ${MAGNUM_EMSCRIPTENAPPLICATION_JS}
            ${MAGNUM_WEBAPPLICATION_CSS}
            DESTINATION ${MAGNUM_DEPLOY_PREFIX})
    install(FILES
            $<TARGET_FILE_DIR:MagnumGameApp>/MagnumGameApp.js.mem
            $<TARGET_FILE_DIR:MagnumGameApp>/MagnumGameApp.wasm
            DESTINATION ${MAGNUM_DEPLOY_PREFIX} OPTIONAL)
endif ()