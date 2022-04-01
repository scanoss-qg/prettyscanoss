package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"os/exec"

	adjust "scanoss.com/scanosslink/tools"
)

type ResultJson struct {
	Key         string   `json:"key,omitempty,omitempty"`
	ID          string   `json:"id,omitempty"`
	Status      string   `json:"status,omitempty"`
	Lines       string   `json:"lines,omitempty"`
	OssLines    string   `json:"oss_lines,omitempty"`
	Snippets    []string `json:"snippets,omitempty"`
	Matched     string   `json:"matched,omitempty"`
	Purl        []string `json:"purl,omitempty"`
	Vendor      string   `json:"vendor,omitempty"`
	Component   string   `json:"component,omitempty"`
	Version     string   `json:"version,omitempty"`
	Latest      string   `json:"latest,omitempty"`
	URL         string   `json:"url,omitempty"`
	ReleaseDate string   `json:"release_date,omitempty"`
	File        string   `json:"file,omitempty"`
	URLHash     string   `json:"url_hash,omitempty"`
	FileHash    string   `json:"file_hash,omitempty"`
	SourceHash  string   `json:"source_hash,omitempty"`
	FileURL     string   `json:"file_url,omitempty"`
	Licenses    []struct {
		Name             string `json:"name,omitempty"`
		PatentHints      string `json:"patent_hints,omitempty"`
		Copyleft         string `json:"copyleft,omitempty"`
		ChecklistURL     string `json:"checklist_url,omitempty"`
		IncompatibleWith string `json:"incompatible_with,omitempty,omitempty"`
		OsadlUpdated     string `json:"osadl_updated,omitempty"`
		Source           string `json:"source,omitempty"`
		Text             string `json:"license_text,omitempty"`
	} `json:"licenses,omitempty"`
	Server struct {
		Version   string `json:"version,omitempty"`
		KbVersion struct {
			Monthly string `json:"monthly,omitempty"`
			Daily   string `json:"daily,omitempty"`
		} `json:"kb_version,omitempty"`
	} `json:"server,omitempty"`
}
type ProcessJob struct {
	Result []ResultJson
	key    string
}

func oss_lines(ranges []adjust.Range) string {
	var rangeStr string
	rangeStr = ""
	for i := range ranges {
		aux := ""
		aux = fmt.Sprintf("%d-%d", ranges[i].RStart, ranges[i].REnd)
		//fmt.Println(aux)
		rangeStr += aux
		if i != (len(ranges) - 1) {
			rangeStr += ", "
		}
	}
	return rangeStr
}
func local_lines(ranges []adjust.Range) string {
	var rangeStr string
	rangeStr = ""
	for i := range ranges {
		aux := ""
		aux = fmt.Sprintf("%d-%d", ranges[i].LStart, ranges[i].LEnd)
		//fmt.Println(aux)
		rangeStr += aux
		if i != (len(ranges) - 1) {
			rangeStr += ", "
		}
	}
	return rangeStr
}
func getGithubLinks(res ResultJson, ranges []adjust.Range) []string {
	var strLinks []string
	for i := range ranges {
		link := fmt.Sprintf("https://github.com/%s/%s/blob/%s/%s#L%d-L%d", res.Vendor, res.Component, res.Version, res.File, ranges[i].RStart, ranges[i].REnd)
		strLinks = append(strLinks, link)

	}

	return strLinks
}

func getLicenseLink(spdx string) string {
	aux := fmt.Sprintf("https://spdx.org/licenses/%s.html", spdx)
	return aux
}

func Wget(url string, filepath string) error {
	// run shell `wget URL -O filepath`
	//	fmt.Printf("downloading %s -> %s\n", url, filepath)
	cmd := exec.Command("wget", url, "-O", filepath)
	//	cmd.Stdout = os.Stdout
	//cmd.Stderr = os.Stderr
	return cmd.Run()
}

func worker(id int, jobs <-chan ProcessJob, resultsChan chan<- ProcessJob) {

	//
	var resAux ProcessJob
	for job := range jobs {
		val := job.Result
		key := job.key

		for i := 0; i < len(val); i++ {
			if val[i].ID != "file" {

				if val[i].FileURL != "" {
					md5Name := val[i].FileHash
					localFile := "../minr/" + key
					remoteFile := "/tmp/" + md5Name
					Wget(val[i].FileURL, remoteFile)
					local, err1 := os.ReadFile(localFile)
					if err1 != nil {
						fmt.Printf("cant load local file %s \n", localFile)
					}
					remote, err2 := os.ReadFile(remoteFile)
					if err2 != nil {
						fmt.Printf("cant load remote file %s \n", remoteFile)
					}
					matches := adjust.Compare(local, remote)
					val[i].OssLines = oss_lines(matches)
					val[i].Snippets = getGithubLinks(val[i], matches) //
					val[i].Lines = local_lines(matches)
				}
			}
			for j := range val[i].Licenses {
				val[i].Licenses[j].Text = getLicenseLink(val[i].Licenses[j].Name)
			}

			//fmt.Println(oss_lines(matches))
			resAux.key = key

			resAux.Result = val
		}
		resultsChan <- resAux
	}

}

func main() {

	x := map[string][]ResultJson{}

	//Parsing/Unmarshalling JSON encoding/json

	err := json.NewDecoder(os.Stdin).Decode(&x)
	if err != nil {
		log.Fatal(err)
	}

	numJobs := len(x)
	jobs := make(chan ProcessJob)
	results := make(chan ProcessJob, numJobs)

	for w := 1; w <= 10; w++ {
		go worker(w, jobs, results)
	}

	for key, val := range x {
		var job ProcessJob
		job.Result = val
		job.key = key
		jobs <- job

	}

	close(jobs)

	for a := 1; a <= numJobs; a++ {
		res := <-results
		//	fmt.Println(res)
		x[res.key] = res.Result
	}
	js, _ := json.Marshal(x)
	fmt.Println(string(js))

}
