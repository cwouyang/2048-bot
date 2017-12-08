package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
)
// Search through evo1.data to evo6.data and print the best evaluation result for
// every evolution. The parameters of best performed evolution is used in final
// program.
func main() {
	for i := 1; i <= 6; i++ {
		file, err := os.Open("evo" + strconv.Itoa(i) + ".data")
		if err != nil {
			return
		}
		defer file.Close()
		fitness := ""
		max_score := -1
		avg_score := -1.0
		max_tile := -1

		scanner := bufio.NewScanner(file)
		scanner.Split(bufio.ScanWords)
		for scanner.Scan() {
			str := scanner.Text()
			if str == "Fittness:" {
				scanner.Scan()
				str = scanner.Text()
				f := str[:(len(str) - 1)]
				scanner.Scan()
				scanner.Text()
				scanner.Scan()
				scanner.Text()
				scanner.Scan()
				str = scanner.Text()
				m_score, _ := strconv.Atoi(str[:(len(str) - 1)])

				scanner.Scan()
				str = scanner.Text()
				scanner.Scan()
				str = scanner.Text()
				scanner.Scan()
				str = scanner.Text()
				a_score, _ := strconv.ParseFloat(str[:(len(str)-1)], 64)

				scanner.Scan()
				str = scanner.Text()
				scanner.Scan()
				str = scanner.Text()
				scanner.Scan()
				str = scanner.Text()
				m_tile, _ := strconv.Atoi(str[:(len(str) - 1)])

				if m_score > max_score && a_score > avg_score && m_tile >= max_tile {
					fitness = f
					max_score = m_score
					avg_score = a_score
					max_tile = m_tile
				}
			}
		}
		fmt.Printf("evo%d, fitness: %s, max_score: %d, avg_score: %f, max_tile: %d\n", i, fitness, max_score, avg_score, max_tile)
	}
}
