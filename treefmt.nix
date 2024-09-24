{ inputs, ... }:
{
  imports = [ inputs.treefmt-nix.flakeModule ];

  perSystem =
    { pkgs, ... }:
    {
      treefmt = {
        # Used to find the project root
        projectRootFile = ".git/config";

        programs.mypy.enable = true;
        programs.nixfmt.package = pkgs.nixfmt-rfc-style;
        programs.deadnix.enable = true;
        programs.ruff.format = true;
        programs.rustfmt.enable = true;
        programs.shfmt.enable = true;
        programs.clang-format.enable = true;
        settings.formatter.shfmt.includes = [ "*.envrc" ];
      };
    };
}
