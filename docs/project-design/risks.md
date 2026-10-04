# Cloud Weather Logger — Risks & Unknowns

> Private — gitignored, not part of the public repo. What could go wrong, and
> the plan for each. Note the outcome in the row once it's been confirmed or dealt with.

See also: [project-plan.md](../../project-plan.md) · [learnings.md](../../learnings.md)

| # | Risk / unknown | Phase | Mitigation |
|---|----------------|-------|------------|
| 1 | **VM's IP changes on restart** — the VM can come back with a new address, breaking the Ansible inventory and the broker address the bridge points at | 5 | Check after a reboot. Use the VM's hostname or a fixed address instead of a raw IP |
| 2 | **No extra-disk command in Multipass** | 5 | Loop device (a file the VM treats as a disk) for the data-disk steps. Confirm Multipass really has no attach option first |
| 3 | **3.5" screen may not share SPI cleanly** — many ILI9488 modules never release MISO, which can corrupt the W25Q64's replies on the shared bus | 4 | Leave the screen's MISO unconnected (write-only). Test flash reads with the screen attached |
| 4 | **I2C on the STM32F4 is fiddly** — bus stuck busy, error flags (still applies through the HAL) | 2 | Scope every step on the DHO804; write a bus-recovery routine; check the errata sheet |
| 5 | **t3.micro has 1 GB RAM** for InfluxDB, Grafana, ingest, plus Ubuntu, Docker and the SSM / CloudWatch agents | 9–10 | Measure real usage on the VM first, with the same Compose memory limits. Short-lived proof deploy, so a t3.small (2 GB) costs pennies more if it doesn't fit |
| 6 | **Ansible over SSM** — needs an S3 bucket for file transfers; runs as SSM's own user, not `deploy` | 10 | Work it out at Phase 10; one-off cloud proof only |
| 7 | **Self-hosted runner on a public repo** — GitHub warns that pull requests from forks can run code on the runner's machine (the dev machine) | 8 | Hardware-in-the-loop job runs only on pushes to own branches, never on fork pull requests |
| 8 | **Scope growth** — ~79 h → ~89 h in one planning session (~90 h after the 2026-10-04 re-plan) | all | Stretch stays stretch; new ideas go to their own projects (e.g. OTA → `curriculum/portfolio/portfolio.md`) |
