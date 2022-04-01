package tools

type IndexedChar struct {
	Line int
	Ch   byte
}

type Range struct {
	LStart int
	LEnd   int
	RStart int
	REnd   int
}

func Normalize(originalText []byte) []IndexedChar {

	var tempText []IndexedChar

	line := 1
	for i := 0; i < len(originalText); i++ {
		if originalText[i] == '\n' {
			line++
			continue
		}
		if (originalText[i] == '\r') || (originalText[i] == '\n') || (originalText[i] == ' ') || (originalText[i] == '\t') {
			continue
		}
		var newIndex IndexedChar

		newIndex.Line = line
		newIndex.Ch = originalText[i]
		tempText = append(tempText, newIndex)
	}
	var endIndex IndexedChar

	endIndex.Line = -1
	endIndex.Ch = ' '
	tempText = append(tempText, endIndex)

	return tempText
}

func Compare(local []byte, remote []byte) []Range {

	var ranges []Range
	localIndex := Normalize(local)
	remoteIndex := Normalize(remote)
	lenRemote := len(remoteIndex)
	lenLocal := len(localIndex)
	var j = 0
	var i = 0
	lastRemote := 0

	for i = 0; i < lenLocal-1; i++ {

		for j = 0; j < lenRemote-1; j++ {
			curr_remote := j
			curr_local := i
			lastLocal := i
			lastRemote = j
			for {
				if (curr_remote < lenRemote-1) && (curr_local < lenLocal) && (localIndex[curr_local].Ch == remoteIndex[curr_remote].Ch) {
					//	fmt.Printf("-> %c(%d)-%c(%d)\n", localIndex[curr_local].Ch, localIndex[curr_local].Line, remoteIndex[curr_remote].Ch, remoteIndex[curr_remote].Line)
					lastLocal = curr_local
					lastRemote = curr_remote
					curr_remote++
					curr_local++

				} else {
					curr_local--
					curr_remote--
					break
				}
			}
			if (curr_remote - j) > 25 {
				var sRange Range
				sRange.LStart = localIndex[i].Line
				sRange.LEnd = localIndex[lastLocal].Line
				sRange.RStart = remoteIndex[j].Line
				sRange.REnd = remoteIndex[lastRemote].Line
				ranges = append(ranges, sRange)
				//	fmt.Printf("Local %d - %d Remoto %d - %d\n", localIndex[i].Line, localIndex[lastLocal].Line, remoteIndex[j].Line, remoteIndex[lastRemote].Line)
				i = curr_local
			}
		}
	}
	return ranges

}
