provider "aws" {
    access_key = "${var.access_key}"
    secret_key = "${var.secret_key}"
    region = "eu-central-1"
}

resource "aws_s3_bucket" "bucket" {
    bucket = "ci.autobot.terraform"
    acl = "public-read-write"
}
