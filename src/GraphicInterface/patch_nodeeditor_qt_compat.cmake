# [TEE patch] Idempotent post-download patches for the goeblr/nodeeditor
# dependency (see CMakeLists.txt's ExternalProject_Add(NodeEditor ...)),
# fixing two incompatibilities between this ~2020-era library and the
# Qt5.15 + TBB combination on this system. Runs as this ExternalProject's
# PATCH_COMMAND (after each fresh clone, so both fixes survive a
# from-scratch build_gui/ regardless of git history); each fix is
# idempotent so a re-run against an already-patched checkout is a no-op.

# 1) QStringStdHash.hpp unconditionally defines std::hash<QString>, but Qt
#    >= 5.14 already provides that specialization itself
#    (QT_SPECIALIZE_STD_HASH_TO_CALL_QHASH_BY_CREF in qhashfunctions.h) ->
#    "redefinition of struct std::hash<QString>" on any Qt5 >= 5.14 (this
#    system has 5.15.18). Wraps the specialization so it only compiles
#    against older Qt5.
set(f "${NODEEDITOR_SOURCE_DIR}/include/nodes/internal/QStringStdHash.hpp")
file(READ "${f}" contents)
if(NOT contents MATCHES "QT_VERSION_CHECK")
  string(REPLACE "namespace std" "#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)\nnamespace std" contents "${contents}")
  string(APPEND contents "\n#endif\n")
  file(WRITE "${f}" "${contents}")
endif()

# 2) SUPRA_GUI compiles with -DQT_NO_KEYWORDS (see this directory's
#    CMakeLists.txt: Qt's emit/signals/slots macros collide with an
#    identically-named method in TBB's profiling.h, pulled in transitively
#    by SupraLib headers). Any translation unit that includes NodeEditor's
#    public headers alongside those needs NodeEditor's own bare
#    emit/signals/slots converted to Q_EMIT/Q_SIGNALS/Q_SLOTS too --
#    otherwise disabling the keyword macros breaks NodeEditor's own class
#    declarations instead. Only these five headers use them (confirmed by
#    grepping the checkout; NodeGraphicsObject.cpp's one "emit" match is
#    inside a comment).
foreach(relpath
    include/nodes/internal/Connection.hpp
    include/nodes/internal/FlowScene.hpp
    include/nodes/internal/FlowView.hpp
    include/nodes/internal/NodeDataModel.hpp
    include/nodes/internal/Node.hpp)
  set(f "${NODEEDITOR_SOURCE_DIR}/${relpath}")
  file(READ "${f}" contents)
  if(NOT contents MATCHES "Q_SIGNALS" AND NOT contents MATCHES "Q_SLOTS")
    string(REGEX REPLACE "\nsignals:" "\nQ_SIGNALS:" contents "${contents}")
    string(REGEX REPLACE "\n([ \t]*)(public|private|protected)[ \t]+slots:" "\n\\1\\2 Q_SLOTS:" contents "${contents}")
    file(WRITE "${f}" "${contents}")
  endif()
endforeach()
