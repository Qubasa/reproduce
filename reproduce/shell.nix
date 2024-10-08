{
  mkShell,
  reproduce,
  libllvm,
  clang,
  ipdb,
  glibc,
}:
let
  removePkg = pkg: pkg != glibc.static;
in
mkShell {
  nativeBuildInputs = (builtins.filter removePkg reproduce.nativeBuildInputs);
  buildInputs = reproduce.buildInputs ++ [
    clang
    libllvm # forgot
    ipdb # python debugger
  ];
  PYTHONBREAKPOINT = "ipdb.set_trace";
  ASAN_OPTIONS = "detect_leaks=0";
  hardeningDisable = [ "all" ];
  shellHook = ''
    export CLANGD_FLAGS="--query-driver='/nix/store/*-clang*/bin/*'"
    export PATH=$PWD/build/bin:$PATH
  '';
}
