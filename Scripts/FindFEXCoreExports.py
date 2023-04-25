#!/usr/bin/python3
import clang.cindex
from clang.cindex import CursorKind
from clang.cindex import TypeKind
from clang.cindex import TranslationUnit
import sys
from dataclasses import dataclass, field
import subprocess
import logging
logger = logging.getLogger()
logger.setLevel(logging.WARNING)

""" Example usage

- TODO: Some structures could be shared with StructPackVerifier
- TODO: Clang TU can parse multiple headers are the same time. Should reduce parsing time.
- CMake globs would find all the headers and pass them in.
- CMake would also provide all the include paths.
- defines like JEMalloc would be provided by cmake as well.

time ./Scripts/FindFEXCoreExports.py \
    External/FEXCore/include/FEXCore/Config/*.h \
    External/FEXCore/include/FEXCore/Core/*.h \
    External/FEXCore/include/FEXCore/Debug/*.h \
    External/FEXCore/include/FEXCore/fextl/*.h \
    External/FEXCore/include/FEXCore/HLE/*.h \
    External/FEXCore/include/FEXCore/IR/*.h \
    External/FEXCore/include/FEXCore/NetStream.h \
    External/FEXCore/include/FEXCore/Utils/*.h \
    -- -IExternal/FEXCore/include/ \
    -IExternal/fmt/include/ \
    -IFEXHeaderUtils/ \
    -IBuild/include/ \
    -IExternal/robin-map/include/ \
    -DENABLE_JEMALLOC > unittests/FEXCore_api/LoaderAPI.h
"""

# These defines are temporarily defined since python3-clang doesn't yet support these.
# Once this tool gets switched over to C++ then this won't be an issue.

# Expression that references a C++20 concept.
CursorKind.CONCEPTSPECIALIZATIONEXPR = CursorKind(153),

# C++2a std::bit_cast expression.
CursorKind.BUILTINBITCASTEXPR = CursorKind(280)

# a concept declaration.
CursorKind.CONCEPTDECL = CursorKind(604),

FileSymbolsDict = {}

def FindClangArguments(OriginalArguments):
    AddedArguments = ["clang"]
    AddedArguments.extend(OriginalArguments)
    AddedArguments.extend(["-v", "-x", "c++", "-S", "-"])
    Proc = subprocess.Popen(AddedArguments, stderr = subprocess.PIPE, stdin = subprocess.DEVNULL)
    NewIncludes = []
    BeginSearch = False
    while True:
        Line = Proc.stderr.readline().strip()

        if not Line:
            Proc.terminate()
            break

        if (Line == b"End of search list."):
            BeginSearch = False
            Proc.terminate()
            break

        if (BeginSearch == True):
            NewIncludes.append("-I" + Line.decode('ascii'))

        if (Line == b"#include <...> search starts here:"):
            BeginSearch = True

    # Add back original arguments
    NewIncludes.extend(OriginalArguments)
    return NewIncludes

@dataclass
class Namespace:
    Symbols: dict
    DataSymbols: dict
    def __init__(self):
        self.Symbols = {}
        self.DataSymbols = {}

@dataclass
class FileSymbols:
    Namespaces: dict
    def __init__(self):
        self.Namespaces = {}

@dataclass
class ArchDB:
    Parsed: bool
    ArchName: str
    NamespaceScope: list
    CurrentNamespace: str
    TU: TranslationUnit
    Structs: dict
    Unions: dict
    VarDecls: dict
    FieldDecls: list
    def __init__(self, ArchName):
        self.Parsed = True
        self.ArchName = ArchName
        self.NamespaceScope = []
        self.CurrentNamespace = ""
        self.TU = None
        self.Structs = {}
        self.Unions = {}
        self.VarDecls = {}
        self.FieldDecls = []



def SetNamespace(Arch):
    Arch.CurrentNamespace = ""
    for Namespace in Arch.NamespaceScope:
        Arch.CurrentNamespace = Arch.CurrentNamespace + Namespace + "::"

def PushNamespace(Arch, Name):
    Arch.NamespaceScope.append(Name)
    SetNamespace(Arch)

def PopNamespace(Arch):
    Arch.NamespaceScope.pop()
    SetNamespace(Arch)

def HandleCursor(Arch, Cursor):
    if (Cursor.kind.is_invalid()):
        Diags = TU.diagnostics
        for Diag in Diags:
            logging.warning (Diag.format())

        Arch.Parsed = False
        return

    for Child in Cursor.get_children():
        kind = Child.kind

        if (kind == CursorKind.STRUCT_DECL or
            kind == CursorKind.UNION_DECL):
            CursorName = Child.type.spelling

            if (len(CursorName) != 0):
                PushNamespace(Arch, CursorName)

            Arch = HandleCursor(Arch, Child)

            if (len(CursorName) != 0):
                PopNamespace(Arch)
        elif (kind == CursorKind.NAMESPACE):
            # Append namespace
            PushNamespace(Arch, Child.spelling)

            # Handle children
            Arch = HandleCursor(Arch, Child)

            # Pop namespace off
            PopNamespace(Arch)
        elif (kind == CursorKind.CXX_METHOD or
           kind == CursorKind.FUNCTION_DECL):
            HasDefaultVisibility = False
            FromFEXCore = False
            HasBody = False
            for ChildFeatures in Child.get_children():
                if ChildFeatures.kind == CursorKind.COMPOUND_STMT:
                    # If this function declaration of CXX method has a compound statement then it has a body.
                    # We don't want to load these.
                    # eg: `FEX_DEFAULT_VISIBILITY inline void SetupFaultEvaluate() {}`
                    HasBody = True

                if ChildFeatures.kind == CursorKind.VISIBILITY_ATTR:
                    HasDefaultVisibility = ChildFeatures.spelling == "default"

            if "FEXCore" in Child.location.file.name:
                FromFEXCore = True

            if HasDefaultVisibility and FromFEXCore and not HasBody:
                if not Child.location.file.name in FileSymbolsDict:
                    FileSymbolsDict[Child.location.file.name] = FileSymbols()

                if not Arch.CurrentNamespace in FileSymbolsDict[Child.location.file.name].Namespaces:
                    FileSymbolsDict[Child.location.file.name].Namespaces[Arch.CurrentNamespace] = Namespace()

                if not Child.spelling in FileSymbolsDict[Child.location.file.name].Namespaces[Arch.CurrentNamespace].Symbols:
                    FileSymbolsDict[Child.location.file.name].Namespaces[Arch.CurrentNamespace].Symbols[Child.spelling] = Child
                else:
                    Prev = FileSymbolsDict[Child.location.file.name].Namespaces[Arch.CurrentNamespace].Symbols[Child.spelling]
                    if Child.mangled_name != Prev.mangled_name:
                        logging.warning("Duplicated exported symbol by name. {} has both {} and {}".format(Child.spelling, Child.mangled_name,
                            Prev.mangled_name))

            Arch = HandleCursor(Arch, Child)
        elif kind == CursorKind.VAR_DECL:
            # Data symbols are this type.
            # FEXCore has /very/ few of these. Basically mmap and munmap.
            HasDefaultVisibility = False
            FromFEXCore = False

            if "FEXCore" in Child.location.file.name:
                FromFEXCore = True

            for ChildFeatures in Child.get_children():
                if ChildFeatures.kind == CursorKind.VISIBILITY_ATTR:
                    HasDefaultVisibility = ChildFeatures.spelling == "default"

            if HasDefaultVisibility and FromFEXCore:
                if not Child.location.file.name in FileSymbolsDict:
                    FileSymbolsDict[Child.location.file.name] = FileSymbols()

                if not Arch.CurrentNamespace in FileSymbolsDict[Child.location.file.name].Namespaces:
                    FileSymbolsDict[Child.location.file.name].Namespaces[Arch.CurrentNamespace] = Namespace()

                if not Child.spelling in FileSymbolsDict[Child.location.file.name].Namespaces[Arch.CurrentNamespace].DataSymbols:
                    FileSymbolsDict[Child.location.file.name].Namespaces[Arch.CurrentNamespace].DataSymbols[Child.spelling] = Child
                else:
                    Prev = FileSymbolsDict[Child.location.file.name].Namespaces[Arch.CurrentNamespace].DataSymbols[Child.spelling]
                    if Child.mangled_name != Prev.mangled_name:
                        logging.warning("Duplicated exported data symbol by name. {} has both {} and {}".format(Child.spelling, Child.mangled_name,
                            Prev.mangled_name))
        else:
            Arch = HandleCursor(Arch, Child)

    return Arch


def GetDB(Arch, filename, args):
    Index = clang.cindex.Index.create()
    try:
        TU = Index.parse(filename, args=args, options=TranslationUnit.PARSE_INCOMPLETE)
    except TranslationUnitLoadError:
        Arch.Parsed = False
        Diags = TU.diagnostics
        for Diag in Diags:
            logging.warning (Diag.format())

        return

    Arch.TU = TU
    HandleCursor(Arch, TU.cursor)

    # Get diagnostics
    Diags = TU.diagnostics
    if (len(Diags) != 0):
        logging.warning ("Diagnostics from Arch: {0}".format(Arch.ArchName))

    for Diag in Diags:
        logging.warning (Diag.format())

    return Arch

def PrintFEXCoreIncludes():
    for FileKey, FileValue in FileSymbolsDict.items():

        # Remove FEXCore prefix
        IncludePath = FileKey.removeprefix("External/FEXCore/include/")

        # Remove `Build/include/` for generated headers
        IncludePath = IncludePath.removeprefix("Build/include/")
        print("#include <{}>".format(IncludePath))

    print("\n")

def PrefixKnownNamespace(Type):
    # If every declaration was fully qualified, this would be unnecessary.
    # return Type.spelling

    if Type.spelling.startswith("fextl::unique_ptr"):
        # If it is a unique_ptr then check the templated types.
        ArgNames = []
        if Type.get_num_template_arguments() > 0:
            for i in range(0, Type.get_num_template_arguments()):
                ArgNames.append(PrefixKnownNamespace(Type.get_template_argument_type(i)))

        return "::fextl::unique_ptr<{}>".format(", ".join(ArgNames))


    if (Type.spelling.startswith("LogMan::") or
        Type.spelling.startswith("FEXCore::") or
        Type.spelling.startswith("fextl::")):
        return "::{}".format(Type.spelling)

    return Type.spelling

def PrintImplForwardDeclares():
    print("#ifdef FEXCORE_FORWARDDECL_DEFINES")

    for FileKey, FileValue in FileSymbolsDict.items():
        print("// Impls from {}".format(FileKey))
        for NamespaceKey, NamespaceValue in FileValue.Namespaces.items():

            # Namespace without the final `::`
            StrippedNamespace = NamespaceKey.rstrip(":").strip()
            EmptyNamespace = len(StrippedNamespace) == 0
            if not EmptyNamespace:
                print("namespace FCL::{} {{".format(StrippedNamespace))
            else:
                print("namespace FCL {")

            for SymbolKey, SymbolValue in NamespaceValue.Symbols.items():

                # Gather the function symbol arguments
                ArgTypes = []
                for Arg in SymbolValue.get_arguments():
                    ArgTypes.append(PrefixKnownNamespace(Arg.type))

                # Create a type for this function symbol first
                print("\tusing {}_Type = {} (*)({});".format(SymbolValue.spelling, PrefixKnownNamespace(SymbolValue.result_type), ", ".join(ArgTypes)))

                # Give a mangled symbol name for this function.
                print("\tconst char {}_sym_name[] = \"{}\";".format(SymbolValue.spelling, SymbolValue.mangled_name))

                # Define the value for this symbol pointer.
                print("\textern {}_Type {};\n".format(SymbolValue.spelling, SymbolValue.spelling))

            for SymbolKey, SymbolValue in NamespaceValue.DataSymbols.items():
                TypeDecl = None
                # Find the type-decl
                for ChildFeatures in SymbolValue.get_children():
                    if ChildFeatures.kind == CursorKind.TYPE_REF:
                        TypeDecl = ChildFeatures

                if type(TypeDecl) == None:
                    logging.error("Data symbol {} was an opaque type?".format(SymbolValue.spelling))
                    sys.exit(1)

                # Give a mangled symbol name for this function.
                print("\tconst char {}_sym_name[] = \"{}\";".format(SymbolValue.spelling, SymbolValue.mangled_name))

                # Define the value for this symbol pointer.
                print("\textern {} {};\n".format(PrefixKnownNamespace(TypeDecl), SymbolValue.spelling))

            print("}")

    # Add the loader declarations
    print("namespace FCL {")

    print("using LoaderFunction = void* (*)(void* UserPtr, const char* SymbolName);")
    print("bool LoadFEXCoreSymbols(LoaderFunction Load);")

    print("}")

    print("#undef FEXCORE_FORWARDDECL_DEFINES")
    print("#endif")

def PrintImplDefs():
    print("#ifdef FEXCORE_IMPL_DEFINES")
    for FileKey, FileValue in FileSymbolsDict.items():
        print("// Impls from {}".format(FileKey))
        for NamespaceKey, NamespaceValue in FileValue.Namespaces.items():
            # Namespace without the final `::`
            StrippedNamespace = NamespaceKey.rstrip(":").strip()
            EmptyNamespace = len(StrippedNamespace) == 0
            if not EmptyNamespace:
                print("namespace FCL::{} {{".format(StrippedNamespace))
            else:
                print("namespace FCL {")

            for SymbolKey, SymbolValue in NamespaceValue.Symbols.items():
                # Define the value for this symbol pointer.
                print("\t{}_Type {};\n".format(SymbolValue.spelling, SymbolValue.spelling))


            for SymbolKey, SymbolValue in NamespaceValue.DataSymbols.items():
                TypeDecl = None
                # Find the type-decl
                for ChildFeatures in SymbolValue.get_children():
                    if ChildFeatures.kind == CursorKind.TYPE_REF:
                        TypeDecl = ChildFeatures

                # Define the value for this symbol pointer.
                print("\t{} {};\n".format(PrefixKnownNamespace(TypeDecl), SymbolValue.spelling))

            print("}")
    print("#undef FEXCORE_IMPL_DEFINES")
    print("#endif")

def PrintLoader():
    print("#ifdef FEXCORE_LOADER")

    print("namespace FCL {")

    print("bool LoadFEXCoreSymbols(LoaderFunction Load, void* UserPtr) {")

    for FileKey, FileValue in FileSymbolsDict.items():
        print("\t// Impls from {}".format(FileKey))
        for NamespaceKey, NamespaceValue in FileValue.Namespaces.items():
            StrippedNamespace = NamespaceKey.strip()
            EmptyNamespace = len(StrippedNamespace) == 0

            for SymbolKey, SymbolValue in NamespaceValue.Symbols.items():
                print("\t{")

                print("\t\tauto SymbolName = FCL::{}{}_sym_name;".format(StrippedNamespace, SymbolValue.spelling))
                print("\t\tFCL::{}{}_Type LoadedSymbol;".format(StrippedNamespace, SymbolValue.spelling))

                print("\t\tLoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, SymbolName));".format(SymbolValue.spelling, SymbolValue.spelling))
                print("\t\tif (LoadedSymbol == nullptr) return false;")
                print("\t\tFCL::{}{} = LoadedSymbol;".format(StrippedNamespace, SymbolValue.spelling))

                print("\t}")


            for SymbolKey, SymbolValue in NamespaceValue.DataSymbols.items():
                TypeDecl = None
                # Find the type-decl
                for ChildFeatures in SymbolValue.get_children():
                    if ChildFeatures.kind == CursorKind.TYPE_REF:
                        TypeDecl = ChildFeatures

                print("\t{")

                print("\t\tauto DataSymbolName = FCL::{}{}_sym_name;".format(StrippedNamespace, SymbolValue.spelling))
                print("\t\t{} LoadedSymbol;".format(PrefixKnownNamespace(TypeDecl)))

                print("\t\tLoadedSymbol = reinterpret_cast<decltype(LoadedSymbol)>(Load(UserPtr, DataSymbolName));".format(SymbolValue.spelling, SymbolValue.spelling))
                print("\t\tif (LoadedSymbol == nullptr) return false;")
                print("\t\tFCL::{}{} = LoadedSymbol;".format(StrippedNamespace, SymbolValue.spelling))

                print("\t}")

    print("\treturn true;")
    print("}")

    print("}")

    print("#undef FEXCORE_LOADER")
    print("#endif")

def main():
    if sys.version_info[0] < 3:
        logging.critical ("Python 3 or a more recent version is required.")

    if (len(sys.argv) < 2):
        print ("usage: %s <Header.hpp> <Header2.hpp...> <...> -- <clang arguments...>" % (sys.argv[0]))

    Headers = []

    StartOfArgs = 0

    # Parse our arguments
    ArgIndex = 1
    while ArgIndex < len(sys.argv):
        Arg = sys.argv[ArgIndex]
        if (Arg == "--"):
            StartOfArgs = ArgIndex + 1
            break;
        else:
            Headers.append(Arg)

        # Increment
        ArgIndex += 1

    BaseArgs = ["-x", "c++", "-std=c++20", "-fvisibility=hidden", "-fvisibility-inlines-hidden"]

    # Add arguments for clang
    for ArgIndex in range(StartOfArgs, len(sys.argv)):
        BaseArgs.append(sys.argv[ArgIndex])

    args_native = []
    args_native.extend(BaseArgs)

    # We need to find the default arguments through clang invocations
    args_native = FindClangArguments(args_native)

    Arch_native = ArchDB("native")

    for Header in Headers:
        Arch_native = GetDB(Arch_native, Header, args_native)

    if not (Arch_native.Parsed):
        logging.critical ("Couldn't parse:{0}".format(Arch_native.ArchName))

    PrintFEXCoreIncludes()
    PrintImplForwardDeclares()
    PrintImplDefs()
    PrintLoader()

    return 0

if __name__ == "__main__":
# execute only if run as a script
    sys.exit(main())
