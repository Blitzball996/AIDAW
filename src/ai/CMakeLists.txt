add_library(aidaw_ai STATIC
    AiEngine.cpp
    MusicAgent.cpp
    VoiceInput.cpp
    PromptParser.cpp
    InstructionExecutor.cpp
    llm/LocalLlmClient.cpp
    llm/CloudLlmClient.cpp
    llm/RelayLlmClient.cpp
    models/ModelManager.cpp
)

target_include_directories(aidaw_ai PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(aidaw_ai PUBLIC
    juce::juce_core
    aidaw_utils
)

if(AIDAW_ENABLE_LOCAL_LLM AND TARGET llama)
    target_link_libraries(aidaw_ai PRIVATE llama)
    target_compile_definitions(aidaw_ai PRIVATE AIDAW_HAS_LOCAL_LLM=1)
endif()
