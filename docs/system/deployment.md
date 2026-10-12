# Deployment

How a server is provisioned, how changes are released, what CI checks, and the
cloud infrastructure.

[← README](../../README.md)

> **Planned, not built.** Nothing on this page exists yet. It is the design the
> project is being built towards, and it may change before it is built.

## Provisioning

```
 first boot:  cloud-init  ─► creates the deploy user + its SSH key
 every run:   Ansible     ─► everything else
```

- One cloud-init file is used by Multipass and as EC2 user data.
- Ansible logs in as `deploy` from its first run and configures the rest.
- The default `ubuntu` user is locked once `deploy` works.

| Environment | Ansible connects over |
|-------------|-----------------------|
| Local VM | SSH |
| EC2 | SSM — no open SSH port, access controlled by IAM |

Only the inventory's connection settings differ; the playbook is the same.

Ansible is run from the Ubuntu development machine. For automated cloud
releases a GitHub Actions runner runs it instead.

## Images

| Environment | Images |
|-------------|--------|
| Local | Built locally |
| Cloud | Built and pushed to ECR by CI; EC2 pulls by tag via its instance role |

## Releases

```
 release.sh <env> <tag>
    1. build the image
    2. store it (local) or push it to ECR (cloud)
    3. run the playbook with that environment's inventory and the new tag
```

- An update is a new image tag plus a playbook re-run. The tag is an Ansible
  variable; the re-run pulls the new image and restarts only the changed
  container.
- The same script is used for both environments. Locally it is run by hand; for
  the cloud, GitHub Actions runs it.
- In the cloud, GitHub Actions gets AWS access via OIDC — no stored keys.

## CI (GitHub Actions)

| Job | Checks |
|-----|--------|
| pytest | Ingest, and the BME280 compensation host tests |
| Firmware build | The firmware compiles |
| ansible-lint | The playbook |
| docker build | The images build |
| Hardware-in-the-loop | The firmware runs on the real board |

**Hardware-in-the-loop:** a self-hosted runner on the development machine,
with the Nucleo plugged in, flashes each new firmware build, sends it commands
and checks the replies.

## Cloud infrastructure (AWS CDK, Python)

| Resource | Purpose |
|----------|---------|
| IoT Core things + policies | Broker identities for the device and for ingest, least privilege |
| EC2 instance | Runs the stack; managed through SSM |
| EBS data volume | InfluxDB data, separate from the root disk |
| ECR repository | Images pushed by CI |
| CloudWatch | Monitoring |

## Grafana access in the cloud

```
aws ssm start-session  ─►  EC2 port 3000  ─►  localhost:3000
```

Grafana binds to `127.0.0.1` on EC2 and the security group has no inbound
rules.

## See also

[architecture.md](architecture.md) · [server.md](server.md) · [firmware.md](firmware.md)
