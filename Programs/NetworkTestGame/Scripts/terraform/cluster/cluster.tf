terraform {
    backend "s3" {
        bucket = "ci.autobot.terraform"
        key    = "terraform.tfstate"
        region = "eu-central-1"
    }
}

provider "aws" {
    access_key = "${var.access_key}"
    secret_key = "${var.secret_key}"
    region = "eu-west-1"
}

provider "aws" {
    alias = "us"
    access_key = "${var.access_key}"
    secret_key = "${var.secret_key}"
    region = "us-west-2"
}

resource "aws_instance" "eu_c48xlarge" {
    count = 2
    ami = "ami-8b66d8f2"
    instance_type = "c4.8xlarge"
    key_name = "${var.key_name}"
}
