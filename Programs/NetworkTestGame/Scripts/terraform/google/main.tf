provider "google" {
    region = "us-central1"
    project = "dava-network-game"
    credentials = "${file(var.account_file_path)}"
}

resource "google_compute_firewall" "allow-all" {
  name    = "allow-all"
  network = "default"

  allow {
    protocol = "udp"
    ports    = ["1-65535"]
  }

  allow {
    protocol = "tcp"
    ports    = ["1-65535"]
  }
}

resource "google_compute_instance" "us_m4large" {
  count = 1
  name = "us"
  machine_type = "n1-highcpu-2"
  zone = "us-west1-a"
  boot_disk {
    initialize_params {
      image = "centos-7-v20171025"
    }
  }

  network_interface {
    network = "default"
    access_config {
      // Ephemeral IP
    }
  }

   metadata_startup_script = <<SCRIPT
sudo yum install iperf3 -y
sudo iperf3 -s
   SCRIPT
}

resource "google_compute_instance" "eu_m4large" {
  count = 2
  name = "eu-${count.index}"
  machine_type = "n1-highcpu-2"
  zone = "europe-west3-a"
  boot_disk {
    initialize_params {
      image = "centos-7-v20171025"
    }
  }

  network_interface {
    network = "default"
    access_config {
      // Ephemeral IP
    }
  }

   metadata_startup_script = <<SCRIPT
sudo yum install iperf3 -y
sudo iperf3 -s
   SCRIPT
}