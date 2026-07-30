include(FetchContent)

FetchContent_Declare(
    FTXUI
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG        v7.0.1
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    cpptypr
    GIT_REPOSITORY https://github.com/AhmedowM/cpptypr.git
GIT_TAG v0.5.1
)

FetchContent_MakeAvailable(FTXUI)
FetchContent_MakeAvailable(cpptypr)
