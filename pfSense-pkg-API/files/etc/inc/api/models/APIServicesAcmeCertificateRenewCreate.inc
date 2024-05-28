<?php
//   Copyright 2023 Jared Hendrickson
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

require_once("api/framework/APIModel.inc");
require_once("api/framework/APIResponse.inc");
require_once("acme/acme.inc");
require_once("acme/acme_utils.inc");

class APIServicesAcmeCertificateRenewCreate extends APIModel {
    private $current_certificates;

    # Create our method constructor
    public function __construct() {
        parent::__construct();
        $this->packages = ["pfSense-pkg-acme"];
        $this->change_note = "Renewed new certificate(s) via API.";
        $this->privileges = ["page-all", "page-service-acme"];
    }

    public function validate_payload() {
        $this->__validate_certificates();
    }

    private function __validate_certificates() {
        foreach ($this->initial_data['certificates'] as $certificate) {
            if (!pfsense_pkg\acme\get_certificate_id($certificate)) {
                $this->errors(2293, ['certificate' => $certificate]);
	    } else {
		$this->validated_data[] = $certificate;
            }
	}
    }

    public function action() {
	foreach ($this->validated_data as $certificate) {
	    ob_start();
            $result = pfsense_pkg\acme\issue_certificate($certificate, true, true);
	    $output = ob_get_clean();
	    if (!$result) {
		$this->errors[] = APIResponse\get(2292, ['certificate' => $certificate], 'acme_output' => $output);
	    }
        }

	return APIResponse\get(0, ['msg' => 'Certificate(s) successfully renewed', 'certificates' => $this->validated_data]);
    }
}
