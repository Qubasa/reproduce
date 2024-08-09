{
  llvmPackages,
  cmake,
  lld,
  pytest,
  glibc,
  pytest-subprocess,
  pytest-xdist,
}:

let
  testDependencies = [
    pytest
    pytest-subprocess
    pytest-xdist
  ];
in
llvmPackages.stdenv.mkDerivation {
  name = "repro-hook";

  src = ./.;

  nativeBuildInputs = [
    glibc.static
    pytest
    cmake
    lld
  ] ++ testDependencies;

  hardeningDisable = [ "all" ];

  # set to cmake release type
  cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" ];

  checkPhase = ''
    ./repro-hook pytest -s
  '';

  doCheck = false; # Ensure that the checkPhase is run
  passthru.testDependencies = testDependencies;
}
